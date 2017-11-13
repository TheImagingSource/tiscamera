#!/usr/bin/env python2

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
import time

from list_devices import select_camera
from list_formats import select_format

import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst, GLib

import cv2
import numpy as np

# Small helper function to display opencv buffers via GStreamer.
#
# The display_buffers list holds references to the buffers we pass down to
# Gstreamer. This is required because otherwise python would free the memory
# before we displayed it and we do not want to copy the data to an allocated
# buffer.
display_buffers = []
def show_img(display_input, img):
    global display_buffers

    bytebuffer = img.tobytes()
    display_buffers.append(bytebuffer)
    new_buf = Gst.Buffer.new_wrapped_full(Gst.MemoryFlags.READONLY, bytebuffer, len(bytebuffer), 0, None, lambda x: display_buffers.pop(0))
    display_input.emit("push-buffer", new_buf)

def callback(sink, display_input):
    """
    This function will be called in a separate thread when our appsink
    says there is data for us."""
    sample = sink.emit("pull-sample")


    if sample:
        buf = sample.get_buffer()

        caps = sample.get_caps()
        width = caps[0].get_value("width")
        height = caps[0].get_value("height")

        try:
            res, mapinfo = buf.map(Gst.MapFlags.READ)
            # actual image buffer and size
            # data = mapinfo.data
            # size = mapinfo.size

            # Create a numpy array from the data
            img_array = np.asarray(bytearray(mapinfo.data), dtype=np.uint8)
            # Give the array the correct dimensions of the video image
            img = img_array.reshape((height, width))
            # Perform opencv operations on the image data
            img = cv2.medianBlur(img, 5)
            th = cv2.adaptiveThreshold(img, 255, cv2.ADAPTIVE_THRESH_MEAN_C, cv2.THRESH_BINARY, 11, 2)

            # Show the result via our display pipeline
            show_img(display_input, th)

        finally:
            buf.unmap(mapinfo)

    return Gst.FlowReturn.OK


def main():
    Gst.init(sys.argv)  # init gstreamer

    # We create a source element to retrieve a device list through it
    source = Gst.ElementFactory.make("tcambin")

    serial = select_camera(source)

    if serial is not None:
        source.set_property("serial", serial)

    # Define the format of the video buffers that will get passed to opencv
    TARGET_FORMAT = "video/x-raw,width=640,height=480,format=GRAY8"

    # Ask the user for the format that should be used for capturing
    fmt = select_format(source.get_by_name("tcambin-source"))
    # If the user selected a bayer format, we change it to BGRx so that the
    # tcambin will decode the bayer pattern to a color image
    if fmt.get_name() == "video/x-bayer":
        fmt.set_name("video/x-raw")
        fmt.set_value("format", "BGRx")
    # Use a capsfilter to determine the video format of the camera source
    capsfilter = Gst.ElementFactory.make("capsfilter")
    capsfilter.set_property("caps", Gst.Caps.from_string(fmt.to_string()))
    # Add a small queue. Everything behind this queue will run in a separate
    # thread.
    queue = Gst.ElementFactory.make("queue")
    queue.set_property("leaky", True)
    queue.set_property("max-size-buffers", 2)
    # Add a videoconvert and a videoscale element to convert the format of the
    # camera to the target format for opencv
    convert = Gst.ElementFactory.make("videoconvert")
    scale = Gst.ElementFactory.make("videoscale")
    # Add an appsink. This element will receive the converted video buffers and
    # pass them to opencv
    output = Gst.ElementFactory.make("appsink")
    output.set_property("caps", Gst.Caps.from_string(TARGET_FORMAT))
    output.set_property("emit-signals", True)
    pipeline = Gst.Pipeline.new()

    # Add all elements
    pipeline.add(source)
    pipeline.add(capsfilter)
    pipeline.add(queue)
    pipeline.add(convert)
    pipeline.add(scale)
    pipeline.add(output)

    # Link the elements
    source.link(capsfilter)
    capsfilter.link(queue)
    queue.link(convert)
    convert.link(scale)
    scale.link(output)


    # Usually one would use cv2.imgshow(...) to display an image but this is
    # tends to hang in threaded environments. So we create a small display
    # pipeline which we could use to display the opencv buffers.
    display_pipeline = Gst.parse_launch("appsrc name=src ! videoconvert ! ximagesink")
    display_input = display_pipeline.get_by_name("src")
    display_input.set_property("caps", Gst.Caps.from_string(TARGET_FORMAT))
    output.connect("new-sample", callback, display_input)
    display_pipeline.set_state(Gst.State.PLAYING)

    pipeline.set_state(Gst.State.PLAYING)

    print("Press Ctrl-C to stop")
    loop = GLib.MainLoop()
    try:
        loop.run()
    except KeyboardInterrupt:
        print("Ctrl-C pressed, terminating")

    # this stops the pipeline and frees all resources
    pipeline.set_state(Gst.State.NULL)

if __name__ == "__main__":
    main()
