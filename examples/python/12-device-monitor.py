#!/usr/bin/env python3

# Copyright 2019 The Imaging Source Europe GmbH
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
# This example will show you how to react to messages on the gstreamer bus.
#

import sys
import re
import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst, GLib


loop = GLib.MainLoop()


def device_action(bus, message, user_data):
    """"""

    if message.type == Gst.MessageType.DEVICE_ADDED:
        device = message.parse_device_added()
        name = device.get_display_name()
        print("Device added: ", name)
        caps = device.get_caps()
        print("\tcaps: ", caps.to_string())
    elif message.type == Gst.MessageType.DEVICE_REMOVED:
        device = message.parse_device_removed()
        name = device.get_display_name()
        print("Device removed: ", name)
    else:
        print(message)
    return True


def setup_source_device_monitor():
    """
    """
    monitor = Gst.DeviceMonitor.new()
    bus = monitor.get_bus()
    bus.add_watch(GLib.PRIORITY_DEFAULT, device_action, None)

    #
    # This filter catches unwanted messages
    # We do not want v4l2src notifications
    # which is why we have tcam in the filter.
    # We want all possible devices and thus do not filter
    # for specific caps.
    monitor.add_filter("Video/Source/tcam", None)

    monitor.start()

    return monitor


def main():

    Gst.init(sys.argv)

    monitor = setup_source_device_monitor()

    # we work with a event loop to be automatically
    # be notified when new messages occur.
    loop.run()

    monitor.stop()


if __name__ == "__main__":
    main()
