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
# This example will show you how to receive data from gstreamer in your application
# and how to get the actual iamge data
#


import sys
import gi
import time

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst


def print_help():
    print("Show how to use appsink to receive images in an appliaction.\n"
          "Usage:\n\appsink [<serial>]\n"
          "Help options:\n\t-h, --help\t\tPrint this text.\n"
          "\n\n")


def select_camera(source):
    """
    Prompt user to select a camera.
    Return the serial of the selected camera.
    Return None on error/abort
    """

    # retrieve all available serial numbers
    serials = source.get_device_serials()

    # create a list to have an easy index <-> serial association
    device_list = []
    # we add None to have a default value for the case 'serial not defined'
    # this also pushes our first serial index to 1.
    device_list.append(None)

    print("Available devices:")
    index = 1
    print("0 - Use default device")

    for s in serials:

        device_list.append(s)
        print("{} - {}".format(index, s))
        index = index + 1

    # get input from user and only stop asking when
    # input is legal
    legal_input = False
    while not legal_input:
        selection = int(input("Please select a device: "))
        if 0 <= selection < len(device_list):
            legal_input = True
        else:
            print("Please select a legal device.")

    return device_list[selection]


def callback(sink):
    """
    This function will be called in a separate thread when our appsink
    says there is data for us."""
    sample = sink.emit("pull-sample")

    if sample:
        # fill the line instead of only having a '*' per line
        print("*", end="", flush=True)

        buf = sample.get_buffer()

        # should you need the caps
        # caps = sample.get_caps()

        try:
            res, mapinfo = buf.map(Gst.MapFlags.READ)

            # actual image buffer and size
            # data = mapinfo.data
            # size = mapinfo.size

        finally:
            buf.unmap(mapinfo)

    return Gst.FlowReturn.OK


def main():
    Gst.init(sys.argv)  # init gstreamer
    serial = None

    if len(sys.argv) > 1:
        if "-h" in sys.argv[1] or "--help" in sys.argv[1]:
            print_help()
            return
        #serial = sys.argv[1]

    # we create a source element to retrieve a device list through it
    source = Gst.ElementFactory.make("tcambin")

    # The user has not given a serial, so we prompt for one
    if serial is None:
        serial = select_camera(source)
        if serial is None:
            print("We will use the default device.")

    # serial is defined, thus make the source open that device
    if serial is not None:
        source.set_property("serial", serial)

    convert = Gst.ElementFactory.make("videoconvert")
    output = Gst.ElementFactory.make("appsink")
    output.set_property("emit-signals", True)
    output.connect("new-sample", callback)
    pipeline = Gst.Pipeline.new()

    pipeline.add(source)
    pipeline.add(convert)
    pipeline.add(output)

    source.link(convert)
    convert.link(output)

    pipeline.set_state(Gst.State.PLAYING)

    wait = True
    while wait:
        input_text = input("Press q<enter> to stop the stream.")
        if (input_text == "q"):
            break

    # this stops the pipeline and frees all resources
    pipeline.set_state(Gst.State.NULL)

if __name__ == "__main__":
    main()
