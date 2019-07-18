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

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst


def print_properties(camera):
    """
    Print selected properties
    """
    (ret, value,
     min_value, max_value,
     default_value, step_size,
     value_type, flags,
     category, group) = camera.get_tcam_property("Exposure Auto")

    if ret:
        print("Expposure Auto has value: {}".format(value))
    else:
        print("Could not query Exposure Auto")

    (ret, value,
     min_value, max_value,
     default_value, step_size,
     value_type, flags,
     category, group) = camera.get_tcam_property("Gain Auto")

    if ret:
        print("Gain Auto has value: {}".format(value))
    else:
        print("Could not query Gain Auto")

    (ret, value,
     min_value, max_value,
     default_value, step_size,
     value_type, flags,
     category, group) = camera.get_tcam_property("Exposure")

    if ret:
        print("Exposure has value: {}".format(value))
    else:
        print("Could not query Exposure")


def main():

    Gst.init(sys.argv)
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

    camera.set_tcam_property("Exposure Auto", False)
    camera.set_tcam_property("Gain Auto", False)

    camera.set_tcam_property("Exposure", 3000)

    print_properties(camera)

    # cleanup, reset state
    camera.set_state(Gst.State.NULL)


if __name__ == "__main__":
    sys.exit(main())
