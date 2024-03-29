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
# This example will show you how to enable trigger-mode
# and how to trigger images with via software trigger.
#

import sys
import gi
import time

gi.require_version("Tcam", "1.0")
gi.require_version("Gst", "1.0")
gi.require_version("GLib", "2.0")

from gi.repository import Tcam, Gst, GLib


def main():

    Gst.init(sys.argv)  # init gstreamer

    # this line sets the gstreamer default logging level
    # it can be removed in normal applications
    # gstreamer logging can contain verry useful information
    # when debugging your application
    # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
    # for further details
    Gst.debug_set_default_threshold(Gst.DebugLevel.WARNING)

    serial = None

    pipeline = Gst.parse_launch("tcambin name=source ! videoconvert ! ximagesink")

    source = pipeline.get_by_name("source")

    # serial is defined, thus make the source open that device
    if serial is not None:
        source.set_property("serial", serial)

    pipeline.set_state(Gst.State.PLAYING)

    # stream for 2 seconds before switching to trigger mode
    # this is simply to show that the device is running
    time.sleep(2)

    try:
        source.set_tcam_enumeration("TriggerMode", "On")

        wait = True
        while wait:
            input_text = input("Press 'Enter' to trigger an image.\n q + enter to stop the stream.")
            if input_text == "q":
                break
            else:
                try:
                    source.set_tcam_command("TriggerSoftware")
                    print("=== Triggered image. ===\n")

                except GLib.Error as err:
                    print("!!! Could not trigger. {}!!!\n", err.message)

        # deactivate trigger mode
        # this is simply to prevent confusion when the camera ist started without wanting to trigger
        source.set_tcam_enumeration("TriggerMode", "Off")

    except GLib.Error as e:

        print(e.message)

    # this stops the pipeline and frees all resources
    pipeline.set_state(Gst.State.NULL)


if __name__ == "__main__":
    main()
