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
# This example will show you how to list available properties
#

import sys
import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst


def list_properties(camera):

    property_names = camera.get_tcam_property_names()

    for name in property_names:

        (ret, value,
         min_value, max_value,
         default_value, step_size,
         value_type, flags,
         category, group) = camera.get_tcam_property(name)

        if not ret:
            print("could not receive value {}".format(name))
            continue

        if value_type == "integer" or value_type == "double":
            print("{}({}) value: {} default: {} min: {} max: {} grouping: {} - {}".format(name,
                                                                                          value_type,
                                                                                          value, default_value,
                                                                                          min_value, max_value,
                                                                                          category, group))
        elif value_type == "string":
            print("{}(string) value: {} default: {} grouping: {} - {}".format(name,
                                                                              value,
                                                                              default_value,
                                                                              category,
                                                                              group))
        elif value_type == "button":
            print("{}(button) grouping is {} -  {}".format(name,
                                                           category,
                                                           group))
        elif value_type == "boolean":
            print("{}(boolean) value: {} default: {} grouping: {} - {}".format(name,
                                                                               value,
                                                                               default_value,
                                                                               category,
                                                                               group))
        elif value_type == "enum":
            enum_entries = camera.get_tcam_menu_entries(name)

            print("{}(enum) value: {} default: {} grouping {} - {}".format(name,
                                                                           value,
                                                                           default_value,
                                                                           category,
                                                                           group))
            print("Entries: ")
            for entry in enum_entries:
                print("\t {}".format(entry))
        else:
            print("This should not happen.")


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
    Gst.init(sys.argv)  # init gstreamer

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

    # set this to a specific camera serial if you
    # do not want to use the default camera
    serial = None

    # get the tcambin to retrieve a property list through it
    source = pipeline.get_by_name("source")

    # serial is defined, thus make the source open that device
    if serial is not None:
        source.set_property("serial", serial)

    print("Properties before state PLAYING:")
    list_properties(source)

    # in the READY state the camera will always be initialized
    # in the PLAYING sta1te additional properties may appear from gstreamer elements
    pipeline.set_state(Gst.State.PLAYING)

    # helper function to ensure we have the right state
    # alternatively wait for the first image
    if not block_until_playing(pipeline):
        print("Unable to start pipeline")

    print("Properties during state PLAYING:")
    list_properties(source)

    pipeline.set_state(Gst.State.NULL)

    return 0


if __name__ == "__main__":
    sys.exit(main())
