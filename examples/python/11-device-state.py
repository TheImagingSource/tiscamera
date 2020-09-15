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
    state = camera.get_property("state")

    print("State of device is:\n{}".format(state))

    # Change JSON description here
    # not part of this example
    camera.set_property("state", state)

    # cleanup, reset state
    camera.set_state(Gst.State.NULL)


if __name__ == "__main__":
    sys.exit(main())
