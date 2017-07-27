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
from .controller import CameraController, CameraNotFoundError

_ = gettext.gettext

PROGNAME = os.path.basename(sys.argv[0])

def handle_list(args):
    def trans_reachable(cam, x):
        if is_reachable(cam):
            return "Yes"
        else:
            return "No"
    def trans_flag(cam, x):
        if not is_reachable(cam):
            return "?"
        if x:
            return "On"
        else:
            return "Off"
    def trans_str(cam, x):
        return str(x)
    def trans_rstr(cam, x):
        if not is_reachable(cam):
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
                if is_reachable(cam):
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
    print("Preparing upload, please wait...")

    ctrl = CameraController()
    ctrl.discover()

    identifier = args["IDENTIFIER"]

    cam = get_camera(identifier, ctrl.cameras)
    if not cam:
        raise CameraNotFoundError

    if not cam["is_reachable"]:
        print(_("The camera is not reachable with the current IP configuration. Aborting."))
        return

    cam = ctrl.get_camera_details(identifier)
    if cam["is_busy"]:
        raise RuntimeError("Camera '%s' is already controlled by a different application" %
                            (cam["serial_number"]))

    _path = os.path.abspath(os.path.realpath(args["FILENAME"]))
    check_fwpath(_path)
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
            try:
                result = camctrl.upload_firmware(cam["serial_number"], fwpath, self.func)
            except IOError as e:
                result = e
            except RuntimeError as e:
                result = e
            self.results.append((cam, result, self.progress))
            self.workqueue.task_done()
        self.progress = None

def _tobytes(value):
    if bytes == str:
        return bytes(value)
    else:
        return bytes(value, "utf-8")

def get_camera(identifier, lst):
    ret = None
    for cam in lst:
        if cam["serial_number"] == identifier:
            ret = cam
        elif cam["user_defined_name"] == identifier:
            ret = cam
        elif cam["mac_address"] == identifier:
            ret = cam
    return ret

def check_fwpath(_path):
    if not os.path.isfile(_path):
        raise IOError("Invalid file: '%s" % (_path))

def is_reachable(cam):
    if 0:
        iface_ip = get_ip_address(cam["interface_name"])
        iface_netmask = get_netmask(cam["interface_name"])
        return address_in_network(iface_ip, cam["current_ip"], iface_netmask)
    else:
        return cam["is_reachable"]

def address_in_network(iface_ip, net_ip, netmask):
   ipaddr = struct.unpack('<L', socket.inet_aton(iface_ip))[0]
   netaddr = struct.unpack('<L', socket.inet_aton(net_ip))[0]
   netmask = struct.unpack('<L', socket.inet_aton(netmask))[0]
   return ipaddr & netmask == netaddr & netmask

def get_ip_address(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x8915,  # SIOCGIFADDR
        struct.pack('256s', _tobytes(ifname[:15]))
    )[20:24])

def get_netmask(ifname):
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    return socket.inet_ntoa(fcntl.ioctl(
        s.fileno(),
        0x891b,  # SIOCGIFNETMASK
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
    iface = args["INTERFACE"]

    print("Discovering cameras on interface '%s'." % (iface,))

    ctrl = CameraController()
    ctrl.discover()

    _path = os.path.abspath(os.path.realpath(args["FILENAME"]))
    check_fwpath(_path)

    cameras = []
    for cam in ctrl.cameras:
        if cam["interface_name"] == iface:
            if is_reachable(cam):
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

    if not args["noconfigure"]:
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
            restxt = "Firmware update FAILED! Upload will be retried (%s)." % (str(result[1]))
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
    KEYS = ["ip", "netmask", "gateway", "name", "mode"]
    identifier = args["IDENTIFIER"]

    ctrl = CameraController()
    ctrl.discover()

    if not get_camera(identifier, ctrl.cameras):
        raise CameraNotFoundError

    res = None

    for key in KEYS:
        if (not key in args) or (args[key] is None):
            continue
        note = ""
        sys.stdout.write(key + ": ")
        value = args[key]
        sys.stdout.write(str(value))

        if key == "mode":
            if args["mode"] == "dhcp":
                res = ctrl.set_persistent_parameter(identifier, "dhcp", 1)
                res = ctrl.set_persistent_parameter(identifier, "static", 0)
                print(" -> OK")
            elif args["mode"] == "static":
                res = ctrl.set_persistent_parameter(identifier, "dhcp", 0)
                res = ctrl.set_persistent_parameter(identifier, "static", 1)
                print(" -> OK")
            elif args["mode"] == "linklocal":
                res = ctrl.set_persistent_parameter(identifier, "dhcp", 0)
                res = ctrl.set_persistent_parameter(identifier, "static", 0)
                print(" -> OK")
            else:
                raise RuntimeError("Invalid mode")
        else:
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
    if is_reachable(cam):
        print("Firmware:".ljust(16) + cam["firmware_version"])
        print("UserName:".ljust(16) + cam["user_defined_name"])
    print("")
    print("MAC Address:".ljust(30) + cam["mac_address"])
    print("Current IP:".ljust(30) + cam["current_ip"])
    print("Current Netmask:".ljust(30) + cam["current_netmask"])
    print("Current Gateway:".ljust(30) + cam["current_gateway"])
    if is_reachable(cam):
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
                            help=_("Unique identifier of the camera: serial number or MAC address"))
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

    subparser = subparsers.add_parser("list", help=_("list connected cameras"))
    subparser.add_argument("--format", type=str, help=_("Format of list"), required=False, nargs="?")
    subparser = subparsers.add_parser("info", help=_("show details of a camera"))
    _add_common_argument(subparser, "i")
    subparser = subparsers.add_parser("set", help=_("permanently set configuration options on the camera"))
    _add_common_argument(subparser, "i")
    subparser.add_argument("--ip", type=str, help=_("IP address to be set"), required=False)
    subparser.add_argument("--netmask", type=str, help=_("netmask to be set"), required=False)
    subparser.add_argument("--gateway", type=str, help=_("gateway address to be set"), required=False)
    subparser.add_argument("--mode", type=str,
                           help=_("IP configuration mode to be set"), required=False,
                           choices=["dhcp", "static", "linklocal"])
    subparser.add_argument("--name", type=str, help=_("set a user defined name"), required=False)
    subparser = subparsers.add_parser("rescue", help=_("remporarily set IP configuration on the camera"))
    _add_common_argument(subparser, "i")
    subparser.add_argument("--ip", type=str, help=_("temporary IP address to be assigned"), required=True)
    subparser.add_argument("--netmask", type=str, help=_("temporary netmask to be assigned"), required=True)
    subparser.add_argument("--gateway", type=str, help=_("temporary gateway address to be assigned"), required=True)
    subparser = subparsers.add_parser("upload", help=_("upload a firmware file to the camera"))
    _add_common_argument(subparser, "i")
    subparser.add_argument("FILENAME", type=str, help=_("filename of firmware file to upload"))
    subparser = subparsers.add_parser("batchupload",
                                      help=_("upload a firmeare file to all cameras connected to a network interface"))
    subparser.add_argument("INTERFACE", type=str, help=_("network interface to scan for cameras"))
    subparser.add_argument("FILENAME", type=str, help=_("filename of firmware file to upload"))
    subparser.add_argument("-n", "--noconfigure", action="store_true", dest="noconfigure",
                           help=_("do not auto-configure IP addresses before upload"), required=False)
    subparser.add_argument("-b", "--baseaddress", type=str,
                           help=_("lowest IP address to use for auto-configurtion (default=x.x.x.10)"), required=False)

    args = vars(parser.parse_args(sys.argv[1:]))
    if args["cmd"]:
        this = sys.modules[__name__]
        handler = getattr(this, "handle_" + args["cmd"])
        try:
            handler(args)
        except CameraNotFoundError:
            print(_("Could not find camera '%s'. Aborting." % (args["IDENTIFIER"])))
        except OSError as e:
            print(e)
    else:
        print(_("Nothing to do. Run '%s -h' for usage information." % (PROGNAME)))


if __name__ == "__main__":
    main()
