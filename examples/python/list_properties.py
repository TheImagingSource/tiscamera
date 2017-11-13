#!/usr/bin/env python3



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
import time

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst

import common

def print_help():
    print("Print properties for device.")
    print("Usage:\n\trigger-images [<serial>]")
    print("Help options:\n\t-h, --help\t\tPrint this text.")
    print("\n\n")


def main():
    Gst.init(sys.argv)  # init gstreamer
    serial = None

    if len(sys.argv) > 1:
        if "-h" in sys.argv[1] or "--help" in sys.argv[1]:
            print_help()
            return
        #serial = sys.argv[1]

    # we create a source element to retrieve a device list through it
    source = Gst.ElementFactory.make("tcambin")

    # The user has not given a serial, so we prompt for one
    if serial is None:
        serial = select_camera(source)
        if serial is None:
            print("We will use the default device.")

    # serial is defined, thus make the source open that device
    if serial is not None:
        source.set_property("serial", serial)

    property_names = source.get_tcam_property_names()

    for name in property_names:

        (ret, value,
         min_value, max_value,
         default_value, step_size,
         value_type, flags,
         category, group) = source.get_tcam_property(name)

        if not ret:
            print("could not receive value {}".format(name))
            continue

        if value_type == "integer" or value_type == "double":
            print("{}(integer) value: {} default: {} min: {} max: {} grouping: {} - {}".format(name,
                                                                                               value, default_value,
                                                                                               min_value, max_value,
                                                                                               category, group))
        elif value_type == "string":
            print("{}(string) value: {} default: {} grouping: {} - {}".format(name, value, default_value, category, group))
        elif value_type == "button":
            print("{}(button) grouping is {} -  {}".format(name, category, group))
        elif value_type == "boolean":
            print("{}(boolean) value: {} default: {} grouping: {} - {}".format(name,
                                                                               value, default_value,
                                                                               category, group))
        elif value_type == "enum":
            enum_entries = source.get_tcam_menu_entries(name)

            print("{}(enum) value: {} default: {} grouping {} - {}".format(name, value,
                                                                           default_value, category, group))
            print("Entries: ")
            for entry in enum_entries:
                print("\t {}".format(entry))
        else:
            print("This should not happen.")

if __name__ == "__main__":
    main()
