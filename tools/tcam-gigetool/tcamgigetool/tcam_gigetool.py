#!/usr/bin/env python3

import time
import sys
import os
import threading
import socket
import fcntl
import struct
import gettext
try:
    import queue
except:
    import Queue as queue

import argparse

from operator import itemgetter

from .version import TCAM_VERSION, TCAM_GIGETOOL_VERSION, TCAM_GIGETOOL_GIT_REVISION
from .controller import CameraController

_ = gettext.gettext

PROGNAME = os.path.basename(sys.argv[0])

def handle_list(args):
    def trans_reachable(cam, x):
        if x:
            return "Yes"
        else:
            return "No"
    def trans_flag(cam, x):
        if not cam["is_reachable"]:
            return "?"
        if x:
            return "On"
        else:
            return "Off"
    def trans_str(cam, x):
        return str(x)
    def trans_rstr(cam, x):
        if not cam["is_reachable"]:
            return "?"
        return str(x)

    namemap = {
        "m": ("model_name", _("Model Name"), 20, trans_str),
        "s": ("serial_number", _("Serial Number"), 16, trans_str),
        "u": ("user_defined_name", _("User Defined Name"), 20, trans_rstr),
        "i": ("current_ip", _("Current IP"), 17, trans_str),
        "n": ("current_netmask", _("Current Netmask"), 17, trans_str),
        "g": ("current_gateway", _("Current Gateway"), 17, trans_str),
        "I": ("persistent_ip", _("Persistent IP"), 17, trans_rstr),
        "N": ("persistent_netmask", _("Persistent Netmask"), 17, trans_rstr),
        "G": ("persistent_gateway", _("Persistent Gateway"), 17, trans_rstr),
        "f": ("interface_name", _("Interface"), 12, trans_str),
        "d": ("is_dhcp_enabled", _("DHCP"), 6, trans_flag),
        "S": ("is_static_ip", _("Static"), 8, trans_flag),
        "M": ("mac_address", _("MAC Address"), 18, trans_str),
        "r": ("is_reachable", _("Reachable"), 10, trans_reachable)
    }
    ctrl = CameraController()
    ctrl.discover()
    ifaces = frozenset([x["interface_name"] for x in ctrl.cameras])
    lst = sorted(ctrl.cameras, key=itemgetter("model_name"))
    lst = sorted(lst, key=itemgetter("serial_number"))
    lst = sorted(lst, key=itemgetter("user_defined_name"))

    fmt_string = args["format"]
    if not fmt_string:
        fmt_string = "%m%s%u%i"

    twidth = None
    for iface in sorted(ifaces):
        s = ""
        i = 0
        while i < len(fmt_string):
            c = fmt_string[i]
            if c == "%":
                i += 1
                c = fmt_string[i]
                if c in namemap:
                    s += namemap[c][1].center(namemap[c][2])
                    if fmt_string[i:].find("%") > 0:
                        s += " | "
                else:
                    s += " "
            else:
                s += " "
            i += 1
        if twidth is None:
            twidth = len(s)-1
        ifacedesc = " Interface: " + iface + " [%s]" % (get_ip_address(iface)) + " "
        print("")
        print(ifacedesc.center(twidth, "-"))
        print("")
        print(s)

        for cam in lst:
            if cam["interface_name"] == iface:
                s = ""
                i = 0
                if cam["is_reachable"]:
                    cam = ctrl.get_camera_details(cam["serial_number"])
                while i < len(fmt_string):
                    c = fmt_string[i]
                    if c == "%":
                        i += 1
                        c = fmt_string[i]
                        if c in namemap:
                            val = namemap[c][3](cam, cam[namemap[c][0]])
                            s += val.ljust(namemap[c][2])
                            if fmt_string[i:].find("%") > 0:
                                s += " | "
                        else:
                            s += c
                    else:
                        s += c
                    i += 1
                print(s)
        print("")

def handle_rescue(args):
    ctrl = CameraController()
    ctrl.discover()

    identifier = args["IDENTIFIER"]
    ip = args["ip"]
    netmask = args["netmask"]
    gateway = args["gateway"]

    return ctrl.rescue(identifier, ip, netmask, gateway)

class FirmwareUploadCallback:
    def __init__(self):
        pass

    def func(self, msg, progress):
        msg = msg.decode("utf-8")
        sys.stdout.write("\r%s%% %s" % (str(progress).rjust(3), msg))

def handle_upload(args):
    ctrl = CameraController()
    ctrl.discover()

    identifier = args["IDENTIFIER"]

    cam = ctrl.get_camera_details(identifier)
    if cam["is_busy"]:
        raise RuntimeError("Camera '%s' is already controlled by a different application" %
                            (cam["serial_number"]))

    _path = os.path.abspath(os.path.realpath(args["FILENAME"]))
    fwupcb = FirmwareUploadCallback()
    res = ctrl.upload_firmware(identifier, _path, fwupcb.func)

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
            result = camctrl.upload_firmware(cam["serial_number"], fwpath, self.func)
            self.results.append((cam, result, self.progress))
            self.workqueue.task_done()
        self.progress = None

def _tobytes(value):
    if bytes == str:
        return bytes(value)
    else:
        return bytes(value, "utf-8")


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

def handle_batchupload(args):
    ctrl = CameraController()
    ctrl.discover()

    iface = args["INTERFACE"]
    _path = os.path.abspath(os.path.realpath(args["FILENAME"]))

    cameras = []
    for cam in ctrl.cameras:
        if cam["interface_name"] == iface:
            if cam["is_reachable"]:
                camdet = ctrl.get_camera_details(cam["serial_number"])
                if camdet["is_busy"]:
                    print("The camera '%s' is busy. Aborting." % (cam["serial_number"]))
                    return
            cameras.append(cam)

    if not cameras:
        print("No cameras found to update")
        return

    if args["baseaddress"]:
        baseip = args["baseaddress"]
        try:
            a,b,c,d = [int(x) for x in baseip.split(".")]
        except ValueError:
            print("Invalid base ip address provided: %s" % (baseip))
            return
    else:
        baseip = get_ip_address(iface)
        try:
            a,b,c,d = [int(x) for x in baseip.split(".")]
        except:
            print("Failed to get IP for interface '%s'" % (iface))
            return
        d = 10
    print("Configuring cameras for address range: {0}.{1}.{2}.{3} .. {0}.{1}.{2}.{4}".format(a,b,c,d,d+len(cameras)-1))
    batchrescue(ctrl, cameras, (a,b,c,d))
    time.sleep(1)

    print("Rediscovering cameras")
    del ctrl
    ctrl = CameraController()
    ctrl.discover(True)
    cameras = []
    for cam in ctrl.cameras:
        if cam["interface_name"] == iface:
            if cam["is_busy"]:
                raise RuntimeError("Camera '%s' is already controlled by a different application" %
                                    (cam["serial_number"]))
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
            res = ctrl.upload_firmware(cam["serial_number"], _path, fwupcb.func)

            if res != 0:
                print ("Upload failed, result: %d" % (res,))

def _parsebool(value):
    TRUES = ["true", "on", "1", "yes"]
    if value.lower() in TRUES:
        return 1
    return 0

def handle_set(args):
    KEYS = ["ip", "netmask", "gateway", "name", "dhcp", "static"]
    EXCLUSIVE = ["dhcp", "static"]
    BOOLARGS = ["dhcp", "static"]
    identifier = args["IDENTIFIER"]

    ctrl = CameraController()
    ctrl.discover()

    res = None

    for key in KEYS:
        if (not key in args) or (args[key] is None):
            continue
        note = ""
        sys.stdout.write(key + ": ")
        value = args[key]
        sys.stdout.write(str(value))

        if key in EXCLUSIVE and value:
            for excl in EXCLUSIVE:
                if excl != key:
                    args[excl] = False
                    KEYS.append(excl)
                    note = "(disabling '%s' because '%s' is enabled)" % (excl, key)

        res = ctrl.set_persistent_parameter(identifier, key, value)
        if res != 0:
            print(" -> FAILED")
            break
        else:
            print(" -> OK " + note)

    if res is None:
        print("Nothing to set.")
    elif res != 0:
        print("Failed to set parameter '%s'" % (key))

def handle_info(args):
    ctrl = CameraController()
    ctrl.discover()

    def bool_to_txt(val):
        if val:
            return "enabled"
        else:
            return "disabled"

    cam = ctrl.get_camera_details(args["IDENTIFIER"])

    print("Model:".ljust(16) + cam["model_name"])
    print("Serial:".ljust(16) + cam["serial_number"])
    if cam["is_reachable"]:
        print("Firmware:".ljust(16) + cam["firmware_version"])
        print("UserName:".ljust(16) + cam["user_defined_name"])
    print("")
    print("MAC Address:".ljust(30) + cam["mac_address"])
    print("Current IP:".ljust(30) + cam["current_ip"])
    print("Current Netmask:".ljust(30) + cam["current_netmask"])
    print("Current Gateway:".ljust(30) + cam["current_gateway"])
    if cam["is_reachable"]:
        print("")
        print("DHCP is: ".ljust(16) + bool_to_txt(cam["is_dhcp_enabled"]))
        print("Static is: ".ljust(16) + bool_to_txt(cam["is_static_ip"]))
        print("")
        print("Persistent IP:".ljust(30) + cam["persistent_ip"])
        print("Persistent Netmask:".ljust(30) + cam["persistent_netmask"])
        print("Persistent Gateway:".ljust(30) + cam["persistent_gateway"])
    else:
        print("\nCamera is currently not reachable.")
        print("Correctly set the IP configuration of the camera to see more information.")


class StoreNameValuePair(argparse.Action):
    def __call__(self, parser, namespace, values, option_string=None):
        n, v = values.split('=')
        setattr(namespace, n, v)

def _add_common_argument(parser, a):
    if a == "i":
        parser.add_argument("IDENTIFIER",
                            type=str,
                            help=_("Unique identifier of the camera: serial number, name or MAC address"))
    elif a == "y":
        parser.add_argument("-y", "--assume-yes",
                            type=bool,
                            help=_("assume 'yes' to all security questions"))

def main():
    TCAM_GIGETOOL_VERSION_DESC = _("tcam-gigetool version:")
    TCAM_GIGETOOL_REVISION_DESC = _("git revision:")
    TCAM_VERSION_DESC = _("tcam library version:")
    fill = max(len(TCAM_GIGETOOL_VERSION_DESC), max(len(TCAM_GIGETOOL_REVISION_DESC), len(TCAM_VERSION_DESC))) + 1
    version_text = TCAM_GIGETOOL_VERSION_DESC.ljust(fill) + TCAM_GIGETOOL_VERSION + "\n"
    version_text += TCAM_GIGETOOL_REVISION_DESC.ljust(fill) + TCAM_GIGETOOL_GIT_REVISION + "\n"
    version_text += TCAM_VERSION_DESC.ljust(fill) + TCAM_VERSION
    help_text = _("Run '%s <COMMAND> -h' to get help on individual commands." % (PROGNAME))
    parser = argparse.ArgumentParser(description=_("The Imaging Source Gigabit Ethernet camera configuration tool"),
                                     formatter_class=argparse.RawDescriptionHelpFormatter,
                                     epilog=(help_text + "\n\nVersion information:\n" + version_text))
    subparsers = parser.add_subparsers(help="subcommand-help", dest="cmd")
    parsers = [("list", _("list connected cameras"), "", 
                [("--format", str, "Format of list", False, "?")]),
               ("info", _("show details of a camera"), "i", []),
               ("set", _("permanently set configuration options on the camera"), "i",
                [
                    ("--ip", str, _("IP address to be set"), False),
                    ("--netmask", str, _("netmask to be set"), False),
                    ("--gateway", str, _("gateway address to be set"), False),
                    ("--static", _parsebool, _("set to static IP"), False, "?"),
                    ("--dhcp", _parsebool, _("enable ip configuration via DHCP"), False, "?"),
                    ("--name", str, _("set a user defined name"), False)
                ]
               ),
               ("rescue", _("remporarily set IP configuration on the camera"), "i",
                [
                    ("--ip", str, _("temporary IP address to be assigned"), True),
                    ("--netmask", str, _("temporary netmask to be assigned"), True),
                    ("--gateway", str, _("temporary gateway address to be assigned"), True)
                ]
               ),
               ("upload", _("upload a firmware file to the camera"), "iy",
                [
                    ("FILENAME", str, _("filename of firmware file to upload"), True),
                ]
               ),
               ("batchupload", _("upload a firmeare file to all cameras connected to a network interface"), "y",
                [
                    ("INTERFACE", str, _("network interface to scan for cameras"), True),
                    ("FILENAME", str, _("filename of firmware file to upload"), True),
                    (["-n", "--noconfigure"], bool, _("do not auto-configure IP addresses before upload"), False),
                    (["-b", "--baseaddress"], bool, _("lowest IP address to use for auto-configurtion (default=x.x.x.10)"), False),
                ]
               )
            ]

    for cmd, helptxt, commonargs, args in parsers:
        subparser = subparsers.add_parser(cmd, help=helptxt)
        for a in commonargs:
            _add_common_argument(subparser, a)
        for a in args:
            is_option = a[0][0].startswith("-")
            action = "store"
            if len(a) > 4:
                nargs = a[4]
            else:
                nargs= None
            if is_option:
                if issubclass(type([]), type(a[0])):
                    subparser.add_argument(*a[0], type=a[1], help=a[2], required=a[3], nargs=nargs)
                else:
                    subparser.add_argument(a[0], type=a[1], help=a[2], required=a[3], nargs=nargs)
            else:
                subparser.add_argument(a[0], type=a[1], help=a[2], nargs=nargs)

    args = vars(parser.parse_args(sys.argv[1:]))
    if args["cmd"]:
        this = sys.modules[__name__]
        handler = getattr(this, "handle_" + args["cmd"])
        handler(args)
    else:
        print(_("Nothing to do. Run '%s -h' for usage information." % (PROGNAME)))


if __name__ == "__main__":
    main()
