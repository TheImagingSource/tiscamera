#!/usr/bin/env python3

# Copyright 2016 The Imaging Source Europe GmbH
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
# This example will show you how to retrieve the available gstreamer
# caps from tcamsrc and how to display print them to stdout.
#


import sys
import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst


def print_formats(source):

    caps = source.get_static_pad("src").query_caps()

    for x in range(caps.get_size()):

        structure = caps.get_structure(x)

        name = structure.get_name()

        fmt = structure.get_value("format")
        # print(type(fmt))

        if type(fmt) is str:
            print("{} {}".format(name, fmt), end="")
        elif type(fmt) is Gst.ValueList:

            print("{} {{ ".format(name), end="")

            for y in range(Gst.ValueList.get_size(fmt)):

                val = Gst.ValueList.get_value(fmt, y)

                print("{} ".format(val), end="")
            print("}", end="")
        else:
            print("==")

        width = structure.get_value("width")
        height = structure.get_value("height")

        print(" - {}x{} - ".format(width, height), end="")

        # the python gobject introspection wrapper
        # can pose problems in older version
        # the types Gst.Fraction and Gst.FractionRange
        # may not be available and thus cause a TypeError
        # in such a case we query the string description
        # of the Gst.Structure and extract the framerates
        try:
            framerates = structure.get_value("framerate")
        except TypeError:
            import re

            substr = structure.to_string()[structure.to_string().find("framerate="):]
            # try for frame rate lists
            field, values, remain = re.split("{|}", substr, maxsplit=3)
            rates = [x.strip() for x in values.split(",")]
            for r in rates:
                print("{} ".format(r), end="")
            print("")
            # we are done here
            continue

        if type(framerates) is Gst.ValueList:

            for y in range(Gst.ValueList.get_size(framerates)):

                val = Gst.ValueList.get_value(framerates, y)

                print("{} ".format(val), end="")

        elif type(framerates) is Gst.FractionRange:

            min_val = Gst.value_get_fraction_range_min(framerates)
            max_val = Gst.value_get_fraction_range_max(framerates)
            print("{} <-> {}".format(min_val, max_val))

        else:
            print("framerates not supported {}".format(type(framerates)))
        # we are finished
        print("")


def main():
    """

    """
    Gst.init(sys.argv)  # init gstreamer

    source = Gst.ElementFactory.make("tcambin")

    # The tcambin wraps the tcamsrc and offers additional
    # formats by implicitly converting
    # source = Gst.ElementFactory.make("tcambin")

    serial = None

    if serial:
        source.set_property("serial", serial)

    source.set_state(Gst.State.READY)

    print_formats(source)

    source.set_state(Gst.State.NULL)


if __name__ == "__main__":
    main()
