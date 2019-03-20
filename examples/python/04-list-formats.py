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


def list_formats(source):
    """
    Returns a list of all video formats supported by a video source.
    """

    # source needs to be at least in READY state to be able to read formats
    old_state = source.get_state(10 * Gst.SECOND)
    if old_state.state == Gst.State.NULL:
        source.set_state(Gst.State.READY)

    caps = source.get_static_pad("src").query_caps()

    # create a list of all structures contained in the caps
    ret = [caps.get_structure(i) for i in range(caps.get_size())]

    # cleanup
    source.set_state(old_state.state)

    return ret


def get_frame_rate_list(fmt):
    """
    Get the list of supported frame rates for a video format.
    This function works arround an issue with older versions of GI that does not
    support the GstValueList type
    """
    try:
        rates = fmt.get_value("framerate")
    except TypeError:
        import re
        # Workaround for missing GstValueList support in GI
        substr = fmt.to_string()[fmt.to_string().find("framerate="):]
        # try for frame rate lists
        _unused_field, values, _unsued_remain = re.split("{|}", substr, maxsplit=3)
        rates = [x.strip() for x in values.split(",")]
    return rates


def select_format(source):
    """
    Helper function that prompts the user to select a video format.

    Returns: Gst.Structure of format
    """
    print("Available formats:")
    i = 0
    formats = list_formats(source)
    for fmt in formats:
        try:
            print("{}: {} ({}x{})".format(i + 1,
                                          fmt.get_value("format"),
                                          fmt.get_value("width"),
                                          fmt.get_value("height")))
            i += 1
        except TypeError:
            # format may contain ANY caps which do not have format/width/height
            pass
    selection = int(input("Select format: "))
    fmt = formats[selection-1]

    print("Available frame rates:")
    frame_rates = get_frame_rate_list(fmt)

    for i in range(len(frame_rates)):

        print("{} : {}".format(i + 1, frame_rates[i]))

    selection = int(input("Select frame rate: "))
    rate = frame_rates[selection-1]

    # work around older GI implementations that
    # lack proper Gst.Fraction/Gst.ValueList support
    if type(rate) == Gst.Fraction:
        fmt.set_value("framerate", rate)
    else:
        numerator, denominator = rate.split("/")
        fmt.set_value("framerate",
                      Gst.Fraction(int(numerator), int(denominator)))

    # fmt is a Gst.Structure but Caps can only be generated from a string,
    # so a to_string conversion is needed
    return fmt


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

            substr = fmt.to_string()[fmt.to_string().find("framerate="):]
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
    serial = "42610001"

    if serial:
        source.set_property("serial", serial)

    source.set_state(Gst.State.READY)

    print_formats(source)

    source.set_state(Gst.State.NULL)


if __name__ == "__main__":
    main()
