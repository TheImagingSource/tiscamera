#!/usr/bin/env python3

# Copyright 2020 The Imaging Source Europe GmbH
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
# This test tests if num-buffers works correctly
#


import sys
import gi
import time

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")
gi.require_version("GstVideo", "1.0")

from gi.repository import Tcam, Gst, GstVideo


framecount = 0
success = False
pipeline = None
required_buffers = 10


def callback(appsink, user_data):
    """
    This function will be called in a separate thread when our appsink
    says there is data for us. user_data has to be defined
    when calling g_signal_connect. It can be used to pass objects etc.
    from your other function to the callback.
    """
    sample = appsink.emit("pull-sample")

    if sample:

        # caps = sample.get_caps()

        gst_buffer = sample.get_buffer()

        try:
            (ret, buffer_map) = gst_buffer.map(Gst.MapFlags.READ)

            global framecount

            # output_str = "Captured frame {}, Pixel Value={} Timestamp={}".format(framecount,
            #                                                                      pixel_data,
            #                                                                      timestamp)

            # print(output_str)

            framecount += 1

        finally:
            gst_buffer.unmap(buffer_map)

    return Gst.FlowReturn.OK


def on_message(self, bus, message):
    t = message.type
    if t == Gst.MessageType.EOS:
        print("Received EOS")
        pipeline.set_state(Gst.State.NULL)

    elif t == Gst.MessageType.ERROR:
        pipeline.set_state(Gst.State.NULL)
        err, debug = message.parse_error()
        print("Error: %s" % err, debug)


def main():

    Gst.init(sys.argv)  # init gstreamer
    serial = None

    pipeline = Gst.parse_launch("tcamsrc name=source num-buffers=10"
                                " ! appsink name=sink")

    # test for error
    if not pipeline:
        print("Could not create pipeline.")
        sys.exit(1)

    # The user has not given a serial, so we prompt for one
    if serial is not None:
        source = pipeline.get_by_name("source")
        source.set_property("serial", serial)

    sink = pipeline.get_by_name("sink")

    # tell appsink to notify us when it receives an image
    sink.set_property("emit-signals", True)

    # tell appsink what function to call when it notifies us
    sink.connect("new-sample", callback, None)

    bus = pipeline.get_bus()
    bus.add_signal_watch()
    bus.connect("message", on_message)

    pipeline.set_state(Gst.State.PLAYING)

    try:
        time.sleep(2)
    except KeyboardInterrupt:
        pass
    finally:
        pipeline.set_state(Gst.State.NULL)

    if required_buffers != framecount:
        print("FAILED expected:{} got: {}".format(required_buffers, framecount))
    else:
        print("SUCCESS")


if __name__ == "__main__":
    main()
