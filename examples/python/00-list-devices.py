#!/usr/bin/env python3

# Copyright 2017 The Imaging Source Europe GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This example will show you how to list information about the available devices
#

import sys
import gi

gi.require_version("Gst", "1.0")
gi.require_version("GLib", "2.0")

from gi.repository import GLib, Gst


def print_device(device):
    """

    """

    # struc is a Gst.Structure
    struc = device.get_properties()

    print("\tmodel:\t{}\tserial:\t{}\ttype:\t{}".format(struc.get_string("model"),
                                                        struc.get_string("serial"),
                                                        struc.get_string("type")))


def bus_function(bus, message, user_data):
    """
    Callback for the GstBus watch
    """

    if message.type == Gst.MessageType.DEVICE_ADDED:
        device = message.parse_device_added()
        print("NEW Device")
        print_device(device)
    elif message.type == Gst.MessageType.DEVICE_REMOVED:
        device = message.parse_device_removed()
        print("REMOVED Device")
        print_device(device)

    return True


if __name__ == "__main__":

    Gst.init(sys.argv)  # init gstreamer

    # this line sets the gstreamer default logging level
    # it can be removed in normal applications
    # gstreamer logging can contain verry useful information
    # when debugging your application
    # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
    # for further details
    Gst.debug_set_default_threshold(Gst.DebugLevel.WARNING)

    monitor = Gst.DeviceMonitor.new()

    monitor.add_filter("Video/Source/tcam")

    #
    # static query
    # list all devices that are available right now
    #

    for device in monitor.get_devices():

        print_device(device)

    #
    # dynamic listing
    # notify us on all device changes (add/remove/changed)
    # all devices will appear once as ADDED
    #

    bus = monitor.get_bus()
    bus.add_watch(GLib.PRIORITY_DEFAULT, bus_function, None)

    monitor.start()
    print("Now listening to device changes. Disconnect your camera to see a remove event. Connect it to see a connect event. Press Ctrl-C to end.\n")

    # This is simply used to wait for events or the user to end this script
    loop = GLib.MainLoop.new(None, False)
    loop.run()

    # has to be called when gst_device_monitor_start has been called
    monitor.stop()
