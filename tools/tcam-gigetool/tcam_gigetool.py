#!/usr/bin/env python3

import time
import sys
import os
import threading
import socket
import fcntl
import struct
try:
    import queue
except:
    import Queue as queue

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

def _tobytes(value):
    if bytes == str:
        return bytes(value)
    else:
        return bytes(value, "utf-8")

class CameraController:
    def __init__(self):
        try:
            self.dll = cdll.LoadLibrary("libtcam_gigewrapper.so")
        except OSError:
            _path = os.path.dirname(__file__)
            if not _path:
                _path = "."
            self.dll = cdll.LoadLibrary(os.path.join(_path,"libtcam_gigewrapper.so"))
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
        self.cameras = []
        return self.dll.get_camera_list(DISCOVER_CALLBACK_FUNC(self.__discover_callback), get_persistent_values)

    def set_persistent_parameter(self, identifier, key, value):
        if type(value) == str:
            return self.dll.set_persistent_parameter_s(_tobytes(identifier),
                                                       _tobytes(key),
                                                       _tobytes(value))
        else:
            return self.dll.set_persistent_parameter_i(_tobytes(identifier),
                                                       _tobytes(key), value)

    def upload_firmware(self, identifier, _path, callback):
        return self.dll.upload_firmware(_tobytes(identifier), _tobytes(_path), callback)

    def get_camera_details(self, identifier):
        cam = TcamCamera()
        self.dll.get_camera_details(_tobytes(identifier), byref(cam))
        return self.__getdict(cam)

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

        self.dll.rescue(_tobytes(mac), _tobytes(ip), _tobytes(netmask), _tobytes(gateway))

def print_usage():
    print("""Usage:
{0} list
    :: Display a list of all available cameras

{0} rescue CAMERA_IDENTIFIER ip=IPADRESS netmask=NETMASK gateway=GATEWAY
    :: Temporarily set the IP address configuration of the camera

{0} set CAMERA_IDENTIFIER [ip=IPADDRESS netmask=NETMASK gateway=GATEWAY static=True | dhcp=True]
    :: Store permanent IP configuration settings

{0} upload CAMERA_IDENTIFIER FIRMWARE_FILE
    :: Upload a firmware file to a camera

{0} batchupload INTERFACE_NAME FIRMWARE_FILE
    :: Auto-configure the IP-Adresses of all cameras detected on the given interface to
       the IP addresses in the subnet, starting with the IP address x.x.x.10 .
       Then start up to 8 threads simultaneously to upload the given firmware file.abs

CAMERA_IDENTIFIER: Can be either the serial number, the user defined name or the MAC address
""".format(sys.argv[0]))


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
    args = sys.argv[3:]
    ip = None
    netmask = None
    gateway = None
    for arg in args:
        key, value = arg.split("=")
        if key == "ip":
            ip = value
        elif key == "netmask":
            netmask = value
        elif key == "gateway":
            gateway = value

    if ip is None or netmask is None or gateway is None:
        print("Argument error")
        return -2

    return ctrl.rescue(identifier, ip, netmask, gateway)

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
        struct.pack('256s', _tobytes(ifname[:15]))
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
    if value.lower() in TRUES:
        return 1
    return 0

def set_persistent():
    BOOLARGS = ["dhcp", "static"]
    EXCLUSIVE = ["dhcp", "static"]
    identifier = sys.argv[2]
    args = sys.argv[3:]

    ctrl = CameraController()
    ctrl.discover()

    for arg in args:
        note = ""
        sys.stdout.write(arg)
        key, value = arg.split("=")
        if key in BOOLARGS:
            value = _parsebool(value)

        if key in EXCLUSIVE and value:
            for excl in EXCLUSIVE:
                if excl != key:
                    args.append("%s=false" % (excl))
                    note = "(disabling '%s' because '%s' is enabled)" % (excl, key)

        res = ctrl.set_persistent_parameter(identifier, key, value)
        if res != 0:
            print(" -> FAILED")
            break
        else:
            print(" -> OK " + note)
    if res != 0:
        print("Failed to set parameter '%s'" % (key))

def show_info():
    ctrl = CameraController()
    ctrl.discover()

    def bool_to_txt(val):
        if val:
            return "enabled"
        else:
            return "disabled"

    identifier = sys.argv[2]
    cam = ctrl.get_camera_details(identifier)

    print("Model:".ljust(16) + cam["model_name"])
    print("Serial:".ljust(16) + cam["serial_number"])
    print("Firmware:".ljust(16) + cam["firmware_version"])
    print("UserName:".ljust(16) + cam["user_defined_name"])
    print()
    print("MAC Address:".ljust(30) + cam["mac_address"])
    print("Current IP:".ljust(30) + cam["current_ip"])
    print("Current Netmask:".ljust(30) + cam["current_netmask"])
    print("Current Gateway:".ljust(30) + cam["current_gateway"])
    print("")
    print("DHCP is: ".ljust(16) + bool_to_txt(cam["is_dhcp_enabled"]))
    print("Static is: ".ljust(16) + bool_to_txt(cam["is_static_ip"]))
    print("")
    print("Persistent IP:".ljust(30) + cam["persistent_ip"])
    print("Persistent Netmask:".ljust(30) + cam["persistent_netmask"])
    print("Persistent Gateway:".ljust(30) + cam["persistent_gateway"])


def main():
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
    elif sys.argv[1] == "info":
        show_info()
    else:
        print_usage()

if __name__ == "__main__":
    main()
