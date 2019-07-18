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


def main():
    Gst.init(sys.argv)  # init gstreamer

    # set this to a specific camera serial if you
    # do not want to use the default camera
    serial = None

    # we create a source element to retrieve a property list through it
    camera = Gst.ElementFactory.make("tcambin")

    # serial is defined, thus make the source open that device
    if serial is not None:
        camera.set_property("serial", serial)

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


if __name__ == "__main__":
    main()
