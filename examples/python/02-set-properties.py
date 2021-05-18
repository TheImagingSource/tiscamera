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
        print("Exposure Auto has value: {}".format(value))
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
     category, group) = camera.get_tcam_property("Brightness")

    if ret:
        print("Brightness has value: {}".format(value))
    else:
        print("Could not query Brightness")


def set_bool_or_enum(source, name, new_value):
    """
    this function basically exists to ensure the example
    works with all camera types.
    If you know the property type of the properties you are
    setting, you can simply call that
    instead of checking the type.

    some settings may exhibit as bool or enum,
    depending on the camera you use.
    """
    (ret, value,
     min_value, max_value,
     default_value, step_size,
     value_type, flags,
     category, group) = source.get_tcam_property(name)

    if not ret:
        print("Could not query property ", name)
        return False

    if value_type == "boolean":
        return source.set_tcam_property(source, name, new_value)
    elif value_type == "enum":

        if new_value:
            new_val = "On"
        else:
            new_val = "Off"

        return source.set_tcam_property(source, name, new_val)


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

    camera.set_tcam_property("Exposure Auto", False)
    camera.set_tcam_property("Gain Auto", False)

    # Some cameras offer exposure and gain as doubles instead of integers.
    # In that case the used GValue type has to be changed when setting the property.
    # Some cameras might offer 'Exposure' as 'Exposure Time (us)'.
    # camera.set_tcam_property("Exposure", 3000.0)
    camera.set_tcam_property("Brightness", 200)

    print_properties(camera)

    # cleanup, reset state
    camera.set_state(Gst.State.NULL)


if __name__ == "__main__":
    sys.exit(main())
