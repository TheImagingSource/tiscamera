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
# This example will show you how to get/set the properties through a description string
#

import sys
import gi

gi.require_version("Gst", "1.0")

from gi.repository import Gst


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
    # gstreamer logging can contain very useful information
    # when debugging your application
    # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
    # for further details
    Gst.debug_set_default_threshold(Gst.DebugLevel.WARNING)

    pipeline = Gst.parse_launch("tcambin name=source ! fakesink")

    if not pipeline:
        print("Unable to create pipeline")
        return 1

    serial = None

    source = pipeline.get_by_name("source")

    # The user has not given a serial, so we prompt for one
    if serial is not None:
        source.set_property("serial", serial)

    # in the READY state the camera will always be initialized
    # in the PLAYING state additional properties may appear from gstreamer elements
    pipeline.set_state(Gst.State.PLAYING)

    if not block_until_playing(pipeline):
        print("Unable to start pipeline")

    # Print properties for a before/after comparison
    state = source.get_property("tcam-properties")

    print(f"State of device is:\n{state.to_string()}")

    # Create new structure
    # containing changes we want to apply

    new_state = Gst.Structure.new_empty("tcam")

    new_state.set_value("ExposureAuto", "Off")
    new_state.set_value("ExposureTime", 35000.0)

    # this can also be done by calling Gst.Structure.from_string()

    source.set_property("tcam-properties", new_state)

    # Print properties for a before/after comparison
    state = source.get_property("tcam-properties")
    print(f"New state of device is:\n{state.to_string()}")

    # cleanup, reset state
    pipeline.set_state(Gst.State.NULL)


if __name__ == "__main__":
    sys.exit(main())
