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

        # video/x-raw, video/x-bayer, etc.
        name = structure.get_name()

        # this is only required when dealing
        # with FPD/MiPi cameras on tegra systems
        features = caps.get_features(x)
        if features:
            if features.contains("memory:NVMM"):
                print("NVMM ")

        try:
            fmt = structure.get_value("format")

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
        except TypeError:  # Gst.ValueList

            # this means we have multiple formats that all
            # have the same width/height/framerate settings

            begin = structure.to_string().find("format=(string){")
            substr = structure.to_string()[begin:]
            values = substr[substr.find("{")+1:substr.find("}")]

            print("{} {{ ".format(name), end="")

            for fmt in values.split(","):

                print("{} ".format(fmt), end="")

            print("}", end="")
            # continue

        # the python gobject introspection wrapper
        # can pose problems in older version
        # the type Gst.IntRange
        # may not be available and thus cause a TypeError
        # in such a case we query the string description
        # of the Gst.Structure and extract the width/height
        try:
            if (structure.to_string().find("width=[") != -1
                    or structure.to_string().find("height=[") != -1):
                raise TypeError

            width = structure.get_value("width")
            height = structure.get_value("height")

            print(" - {}x{} - ".format(width, height), end="")

        except TypeError:

            import re

            # width handling

            begin = structure.to_string().find("width=(int)[")
            substr = structure.to_string()[begin:]
            values = substr[substr.find("[")+1:substr.find("]")]
            v = re.findall(r'\d+', values)

            # assume first entry is smaller
            width_min = v[0]
            width_max = v[1]

            # height handling

            begin = structure.to_string().find("height=(int)[")
            substr = structure.to_string()[begin:]
            values = substr[substr.find("[")+1:substr.find("]")]
            v = re.findall(r'\d+', values)

            height_min = v[0]
            height_max = v[1]

            print(" - {}x{} <=> {}x{} - ".format(width_min, height_min, width_max, height_max), end="")

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

            try:
                # try for frame rate lists
                field, values, remain = re.split("{|}", substr, maxsplit=3)
                rates = [x.strip() for x in values.split(",")]
                for r in rates:
                    print("{} ".format(r), end="")
            except ValueError:  # we have a GstFractionRange

                values = substr[substr.find("[")+1:substr.find("]")]
                v = re.findall(r'\d+', values)
                fps_min_num = v[0]
                fps_min_den = v[1]
                fps_max_num = v[2]
                fps_max_den = v[3]
                # framerates are fractions thus one framerate euqals two values
                print("{}/ {} <=> {}/{}".format(fps_min_num, fps_min_den,
                                                fps_max_num, fps_max_den), end="")

            # printf line break
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

    # this line sets the gstreamer default logging level
    # it can be removed in normal applications
    # gstreamer logging can contain verry useful information
    # when debugging your application
    # see https://gstreamer.freedesktop.org/documentation/tutorials/basic/debugging-tools.html
    # for further details
    Gst.debug_set_default_threshold(Gst.DebugLevel.WARNING)

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
