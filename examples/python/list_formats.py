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

from list_devices import select_camera
import gi

gi.require_version("Tcam", "0.1")
gi.require_version("Gst", "1.0")

from gi.repository import Tcam, Gst

import common

def list_formats(source):
    """Returns a list of all video formats supported by a video source."""

    # source needs to be at least in READY state to be able to read formats
    old_state = source.get_state(10 * Gst.SECOND)
    if old_state.state == Gst.State.NULL:
        source.set_state(Gst.State.READY)

    caps = source.pads[0].query_caps()

    # create a list of all structures contained in the caps
    ret = [caps.get_structure(i) for i in range(caps.get_size())]

    # cleanup
    source.set_state(old_state.state)

    return ret

def get_frame_rate_list(fmt):
    """Get the list of supported frame rates for a video format.
    This function works arround an issue with older versions of GI that does not
    support the GstValueList type"""
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
    """Helper function that prompts the user to select a video format.

    Returns: Gst.Structure of format
    """
    print("Available formats:")
    i = 0
    formats = list_formats(source)
    for fmt in formats:
        try:
            print("%d: %s (%dx%d)" %
                  (i + 1,
                   fmt.get_value("format"), fmt.get_value("width"), fmt.get_value("height")))
            i += 1
        except TypeError:
            # format may contain ANY caps which do not have format/width/height
            pass
    selection = int(input("Select format: "))
    fmt = formats[selection-1]

    print("Available frame rates:")
    frame_rates = get_frame_rate_list(fmt)
    for i in range(len(frame_rates)):
        print("%d : %s" % (i+1, frame_rates[i]))
    selection = int(input("Select frame rate: "))
    rate = frame_rates[selection-1]

    # work around older GI implementations that lack proper Gst.Fraction/Gst.ValueList support
    if type(rate) == Gst.Fraction:
        fmt.set_value("framerate", rate)
    else:
        numerator, denominator = rate.split("/")
        fmt.set_value("framerate", Gst.Fraction(int(numerator), int(denominator)))

    # fmt is a Gst.Structure but Caps can only be generated from a string,
    # so a to_string conversion is needed
    return fmt

if __name__ == "__main__":
    Gst.init(sys.argv)  # init gstreamer
    source = Gst.ElementFactory.make("tcamsrc")
    serial = select_camera(source)
    source.set_property("serial", serial)

    fmt = select_format(source)
    print("Selected format: ", fmt)
