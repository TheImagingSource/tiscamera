#!/usr/bin/env python

import time
import sys
import os
import threading
import socket
import fcntl
import struct
import queue

import argparse

from ctypes import *

from operator import itemgetter

MAX_CAMERAS = 64

class TcamCamera(Structure):
    _fields_ = [("model_name", c_char * 64),
                ("serial_number", c_char * 64),
                ("current_ip", c_char * 16),
                ("current_gateway", c_char * 16),
                ("current_netmask", c_char * 16),
                ("persistent_ip", c_char * 16),
                ("persistent_gateway", c_char * 16),
                ("persistent_netmask", c_char * 16),
                ("user_defined_name", c_char * 64),
                ("firmware_version", c_char * 64),
                ("mac_address", c_char * 64),
                ("interface_name", c_char * 64),
                ("is_static_ip", c_int),
                ("is_dhcp_enabled", c_int),
                ("is_reachable", c_int),
                ("is_controllable", c_int)
    ]

DISCOVER_CALLBACK_FUNC = CFUNCTYPE(None, TcamCamera)
UPLOAD_CALLBACK_FUNC = CFUNCTYPE(None, c_char_p, c_int)

class CameraController:
    def __init__(self):
        _path = os.path.dirname(__file__)
        if not _path:
            _path = "."
        self.dll = cdll.LoadLibrary(os.path.join(_path,"gigewrapper.so"))
        self.dll.init()
        self.dll.set_persistent_parameter_s.argtypes = [c_char_p, c_char_p, c_char_p]
        self.dll.set_persistent_parameter_i.argtypes = [c_char_p, c_char_p, c_int]
        self.dll.rescue.argtypes = [c_char_p, c_char_p, c_char_p, c_char_p]
        self.dll.upload_firmware.argtypes = [c_char_p, c_char_p, UPLOAD_CALLBACK_FUNC]
        self.cameras = []

    SUCCESS = 0x0
    FAILURE = 0x8000
    NO_DEVICE = 0x8001
    INVALID_PARAMETER = 0x8002

    @staticmethod
    def __getdict(struct):
        d = dict((field, getattr(struct, field)) for field, _ in struct._fields_)
        for f in d:
            if type(d[f]) == bytes:
                d[f] = d[f].decode("utf-8")
        return d

    def __discover_callback(self, camera):
        self.cameras.append(self.__getdict(camera))

    def discover(self, get_persistent_values=False):
        self.cameras.clear()
        return self.dll.get_camera_list(DISCOVER_CALLBACK_FUNC(self.__discover_callback), get_persistent_values)

    def set_persistent_parameter(self, identifier, key, value):
        if type(value) == str:
            return self.dll.set_persistent_parameter_s(bytes(identifier, "utf-8"),
                                                       bytes(key, "utf-8"),
                                                       bytes(value, "utf-8"))
        else:
            return self.dll.set_persistent_parameter_i(bytes(identifier, "utf-8"),
                                                       bytes(key, "utf-8"), value)

    def upload_firmware(self, identifier, _path, callback):
        return self.dll.upload_firmware(bytes(identifier, "utf-8"), bytes(_path, "utf-8"), callback)

    def rescue(self, identifier, ip, netmask, gateway):
        mac = None
        for cam in self.cameras:
            if identifier in [cam["serial_number"], cam["user_defined_name"], cam["mac_address"]]:
                if mac is not None:
                    print("Camera identifier is ambiguous")
                    return -2
                mac = cam["mac_address"]

        if mac is None:
            print("No matching camera")
            return -1

        # print("send rescue to: ", mac, ip, netmask, gateway)

        self.dll.rescue(bytes(mac, "utf-8"), bytes(ip, "utf-8"), bytes(netmask, "utf-8"), bytes(gateway, "utf-8"))

def print_usage():
    print("""Usage:
{0} list

{0} upload CAMERA_IDENTIFIER FIRMWARE_FILE

{0} store CAMERA_IDENTIFIER [KEY=VALUE..]

{0} rescue CAMERA_IDENTIFIER ip=IPADRESS netmask=NETMASK gateway=GATEWAY

{0} set CAMERA_IDENTIFIER [ip=IPADDRESS netmask=NETMASK gateway=GATEWAY static=True | dhcp=True]

{0} batchupload FIRMWARE_FILE""".format(sys.argv[0]))


def list():
    ctrl = CameraController()
    ctrl.discover()
    lst = sorted(ctrl.cameras, key=itemgetter("model_name"))
    lst = sorted(lst, key=itemgetter("serial_number"))
    lst = sorted(lst, key=itemgetter("user_defined_name"))
    lst = sorted(lst, key=itemgetter("interface_name"))

    for cam in lst:
        s = (cam["model_name"].ljust(20) +
             cam["serial_number"].ljust(10) +
             cam["user_defined_name"].ljust(20) +
             cam["current_ip"].ljust(17) +
             cam["interface_name"].ljust(12))
        print(s)

def rescue():
    ctrl = CameraController()
    ctrl.discover()

    identifier = sys.argv[2]
    ip = sys.argv[3]
    netmask = sys.argv[4]
    gateway = sys.argv[5]

    ctrl.rescue(identifier, ip, netmask, gateway)
    ctrl.discover()

class FirmwareUploadCallback:
    def __init__(self):
        pass

    def func(self, msg, progress):
        msg = msg.decode("utf-8")
        sys.stdout.write("\r%s%% %s" % (str(progress).rjust(3), msg))

def upload():
    ctrl = CameraController()
    ctrl.discover()

    identifier = sys.argv[2]
    _path = os.path.abspath(os.path.realpath(sys.argv[3]))
    fwupcb = FirmwareUploadCallback()
    res = ctrl.upload_firmware(identifier, _path, UPLOAD_CALLBACK_FUNC(fwupcb.func))

    if res != 0:
        print ("Upload failed, result: %d" % (res,))


class BatchUploadThread(threading.Thread):
    def __init__(self, workqueue):
        threading.Thread.__init__(self)
        self.progress = 0
        self.workqueue = workqueue
        self.results = []

    def func(self, msg, progress):
        msg = msg.decode("utf-8")
        self.progress = progress

    def run(self):
        while not self.workqueue.empty():
            self.progress = 0
            try:
                camctrl, cam, fwpath = self.workqueue.get_nowait()
            except queue.QueueEmpty:
                break
            result = camctrl.upload_firmware(cam["serial_number"], fwpath, UPLOAD_CALLBACK_FUNC(self.func))
            self.results.append((cam, result, self.progress))
            self.workqueue.task_done()
        self.progress = None

def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,  # SIOCGIFADDR
        struct.pack('256s', bytes(ifname[:15], "utf-8"))
    )[20:24])

def batchrescue(ctrl, cameras, baseip):
    ip_a, ip_b, ip_c, ip_d = baseip
    for cam in cameras:
        ip = "%d.%d.%d.%d" % (ip_a, ip_b, ip_c, ip_d)
        netmask = "255.255.255.0"
        gateway = "0.0.0.0"
        ctrl.rescue(cam["serial_number"], ip, netmask, gateway)
        ip_d += 1

def batchupload():
    ctrl = CameraController()
    ctrl.discover()

    iface = sys.argv[2]
    _path = os.path.abspath(os.path.realpath(sys.argv[3]))

    cameras = []
    for cam in ctrl.cameras:
        if cam["interface_name"] == iface:
            cameras.append(cam)

    if not cameras:
        print("No cameras found to update")
        return

    ifaceip = get_ip_address(iface)
    a,b,c,d = [int(x) for x in ifaceip.split(".")]
    d = 10
    print("Configuring cameras for address range: {0}.{1}.{2}.{3} .. {0}.{1}.{2}.{4}".format(a,b,c,d,d+len(cameras)-1))
    batchrescue(ctrl, cameras, (a,b,c,d))
    time.sleep(1)

    print("Rediscovering cameras")
    del ctrl
    ctrl = CameraController()
    ctrl.discover()
    cameras = []
    for cam in ctrl.cameras:
        if cam["interface_name"] == iface:
            cameras.append(cam)

    print("Starting Threads...")
    threads = []
    workqueue = queue.Queue()
    for cam in cameras:
        workqueue.put((ctrl, cam, _path))

    numthreads = min(workqueue.qsize(), 8)
    for t in range(numthreads):
        t = BatchUploadThread(workqueue)
        t.start()
        threads.append(t)

    done = False
    while not done:
        time.sleep(0.5)
        wc = threading.active_count() -1
        remain = workqueue.qsize()
        s = "Working:" + str(wc).ljust(3) + " Remaining:" + str(remain).ljust(3) + " "
        for t in threads:
            if t.progress is None:
                progress = "---".center(6)
            else:
                progress = "%d%%" % (t.progress)
            s += progress.ljust(6)
        sys.stdout.write("\r%s" % (s))
        done = threading.active_count() == 1

    print("\r".ljust(79))

    results = []
    for t in threads:
        results += t.results

    print("Finished. Update summary:")
    failed = []
    for result in results:
        if result[1] == 0:
            restxt = "Update completed successfully"
        else:
            restxt = "Firmware update FAILED! Upload will be retried (%d)." % (result[1])
            failed.append(result[0])
        print("Device: %s, completed: %d%%, result: %s" % (result[0]["serial_number"], result[2], restxt))

    if failed:
        print("At least one device failed to update")
        print("Retrying the upload in sequential mode.")
        del ctrl
        ctrl = CameraController()
        ctrl.discover()
        time.sleep(1)
        for cam in failed:
            print("Uploading to device: %s" % (cam["serial_number"]))
            fwupcb = FirmwareUploadCallback()
            res = ctrl.upload_firmware(cam["serial_number"], _path, UPLOAD_CALLBACK_FUNC(fwupcb.func))

            if res != 0:
                print ("Upload failed, result: %d" % (res,))

def _parsebool(value):
    TRUES = ["true", "on", "1", "yes"]
    if value in TRUES:
        return 1
    return 0

def set_persistent():
    BOOLARGS = ["dhcp", "static"]
    identifier = sys.argv[2]
    args = sys.argv[3:]

    ctrl = CameraController()
    ctrl.discover()

    for arg in args:
        print(arg)
        key, value = arg.split("=")
        if key in BOOLARGS:
            value = _parsebool(value)

        res = ctrl.set_persistent_parameter(identifier, key, value)
        if res != 0:
            break
    if res != 0:
        print("Failed to set parameter '%s'" % (key))

if __name__ == "__main__":
    # parser = argparse.ArgumentParser(description="The Imaging Source Gigabit Ethernet camera configuration tool")
    # parser.add_argument("command", nargs="+", help="command to execute")
    # parser.add_argument("identifier", nargs="?", help="")

    if len(sys.argv) < 2:
        print_usage()
        sys.exit(1)

    if sys.argv[1] == "list":
        list()
    elif sys.argv[1] == "rescue":
        rescue()
    elif sys.argv[1] == "upload":
        upload()
    elif sys.argv[1] == "batchupload":
        batchupload()
    elif sys.argv[1] == "set":
        set_persistent()
    else:
        print_usage()

    # camctl = CameraController()
    # camctl.discover()
    # print(camctl.cameras)
    # #print(hex(camctl.set_persistent_parameter(b"11719912", b"foo", b"bar")))
    # fwupcb = FirmwareUploadCallback()
    # camctl.upload_firmware(b"11719912", b"GigE3Firmware.1672-All.fwpack", UPLOAD_CALLBACK_FUNC(fwupcb.func))
