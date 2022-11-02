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
# This example will show you how to set properties
#

import sys
import gi

gi.require_version("Tcam", "1.0")
gi.require_version("Gst", "1.0")
gi.require_version("GLib", "2.0")

from gi.repository import Tcam, Gst, GLib


def print_properties(camera):
    """
    Print selected properties
    """
    try:

        property_exposure_auto = camera.get_tcam_property("ExposureAuto")

        print(property_exposure_auto.get_value())

        value = camera.get_tcam_enumeration("ExposureAuto")

        print(f"Exposure Auto has value: {value}")

        value = camera.get_tcam_enumeration("GainAuto")

        print("Gain Auto has value: {}".format(value))

        value = camera.get_tcam_float("ExposureTime")

        print("ExposureTimer has value: {}".format(value))

    except GLib.Error as err:

        print(f"{err.message}")


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

    # Set this to a serial string for a specific camera
    serial = None

    camera = Gst.ElementFactory.make("tcambin")

    if serial:
        # This is gstreamer set_property
        camera.set_property("serial", serial)

    # in the READY state the camera will always be initialized
    camera.set_state(Gst.State.READY)

    # Print properties for a before/after comparison
    print_properties(camera)

    # Set properties
    try:
        camera.set_tcam_enumeration("ExposureAuto", "Off")
        camera.set_tcam_enumeration("GainAuto", "Off")

        camera.set_tcam_float("ExposureTime", 2000)

    except GLib.Error as err:
        # if setting properties fail, print the reason
        print(f"{err.message}")

    print_properties(camera)

    # cleanup, reset state
    camera.set_state(Gst.State.NULL)


if __name__ == "__main__":
    sys.exit(main())
