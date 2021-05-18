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
# This example will show you how to save a video stream to a file
#

import time
import sys
import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst


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

    pipeline = Gst.parse_launch("tcambin name=bin"
                                " ! video/x-raw,format=BGRx,width=640,height=480,framerate=30/1"
                                " ! tee name=t"
                                " ! queue"
                                " ! videoconvert"
                                " ! ximagesink"
                                " t."
                                " ! queue"
                                " ! videoconvert"
                                " ! avimux"
                                " ! filesink name=fsink")

    # to save a video without live view reduce the pipeline to the following:

    # pipeline = Gst.parse_launch("tcambin name=bin"
    #                             " ! video/x-raw,format=BGRx,width=640,height=480,framerate=30/1"
    #                             " ! videoconvert"
    #                             " ! avimux"
    #                             " ! filesink name=fsink")

    # serial is defined, thus make the source open that device
    if serial is not None:
        camera = pipeline.get_by_name("bin")
        camera.set_property("serial", serial)

    file_location = "/tmp/tiscamera-save-stream.avi"

    fsink = pipeline.get_by_name("fsink")
    fsink.set_property("location", file_location)

    pipeline.set_state(Gst.State.PLAYING)

    print("Press Ctrl-C to stop.")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        pipeline.set_state(Gst.State.NULL)


if __name__ == "__main__":
    main()
