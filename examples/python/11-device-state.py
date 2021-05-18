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
# This example will show you how to set properties
#

import sys
import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst


def block_until_playing(pipeline):

    while True:
        # wait 0.1 seconds for something to happen
        change_return, state, pending = pipeline.get_state(100000000)
        if change_return == Gst.StateChangeReturn.SUCCESS:
            return True
        elif change_return == Gst.StateChangeReturn.FAILURE:
            print("Failed to change state {} {} {}".format(change_return,
                                                           state,
                                                           pending))
            return False


def main():

    Gst.init(sys.argv)

    # this line sets the gstreamer default logging level
    # it can be removed in normal applications
    # gstreamer logging can contain verry useful information
    # when debugging your application
    # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
    # for further details
    Gst.debug_set_default_threshold(Gst.DebugLevel.WARNING)

    pipeline = Gst.parse_launch("tcambin name=source ! fakesink")

    if not pipeline:
        print("Unable to create pipeline")
        return 1

    # Set this to a serial string for a specific camera
    serial = None

    camera = Gst.ElementFactory.make("tcambin")

    if serial:
        # This is gstreamer set_property
        camera.set_property("serial", serial)

    # in the READY state the camera will always be initialized
    # in the PLAYING state additional properties may appear from gstreamer elements
    pipeline.set_state(Gst.State.PLAYING)

    if not block_until_playing(pipeline):
        print("Unable to start pipeline")

    # Print properties for a before/after comparison
    state = camera.get_property("state")

    print("State of device is:\n{}".format(state))

    # Change JSON description here
    # not part of this example
    camera.set_property("state", state)

    # Print properties for a before/after comparison
    state = camera.get_property("state")
    print("State of device is:\n{}".format(state))

    # cleanup, reset state
    pipeline.set_state(Gst.State.NULL)


if __name__ == "__main__":
    sys.exit(main())
