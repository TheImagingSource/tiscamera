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


def bus_callback(bus, message, user_data):
    """
    Gstreamer Message Types and how to parse
    https://lazka.github.io/pgi-docs/Gst-1.0/flags.html#Gst.MessageType
    """
    mtype = message.type

    if mtype == Gst.MessageType.EOS:
        # end-of-stream
        loop.quit()
    elif mtype == Gst.MessageType.ERROR:
        # Handle Errors
        err, debug = message.parse_error()
        print(err, debug)

        # if you use tcamsrc directly this will be the name you give to the element
        # if (strcmp(source_name, "tcamsrc0") == 0)
        if message.src.get_name() == "tcambin-source":

            if err.message.startswith("Device lost ("):

                m = re.search('Device lost \((.*)\)', err.message)
                print("Device lost came from device with serial = {}".format(m.group(1)))

                # device lost handling should be initiated here
                # this example simply stops playback
                loop.quit()

    elif mtype == Gst.MessageType.WARNING:
        # Handle warnings
        err, debug = message.parse_warning()
        print(err, debug)

    return True


def main():

    Gst.init(sys.argv)

    # this line sets the gstreamer default logging level
    # it can be removed in normal applications
    # gstreamer logging can contain verry useful information
    # when debugging your application
    # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
    # for further details
    Gst.debug_set_default_threshold(Gst.DebugLevel.WARNING)

    serial = None

    pipeline = Gst.parse_launch("tcambin name=source ! videoconvert ! ximagesink")

    if not pipeline:
        print("Could not create pipeline")
        sys.exit(1)

    if serial:
        src = pipeline.get_by_name("source")
        src.set_property("serial", serial)
        src = None

    bus = pipeline.get_bus()
    bus.add_signal_watch()
    bus.enable_sync_message_emission()
    bus.connect('message', bus_callback, None)

    pipeline.set_state(Gst.State.PLAYING)

    print("Disconnect your camera to trigger a device lost or press enter to stop the stream.")

    # we work with a event loop to be automatically
    # be notified when new messages occur.
    loop.run()

    pipeline.set_state(Gst.State.NULL)


if __name__ == "__main__":
    main()
