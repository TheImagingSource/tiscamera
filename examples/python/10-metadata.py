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
# This example will show you how to enable trigger-mode
# and how to trigger images with via software trigger.
#

import sys
import gi
import time
import ctypes

gi.require_version("Gst", "1.0")
gi.require_version("GstVideo", "1.0")
gi.require_version("GObject", "2.0")
gi.require_version("GLib", "2.0")

from gi.repository import Gst, GstVideo, GObject, GLib

# workaround for missing GstMeta apis

# load tiscamera GstMeta library
clib = ctypes.CDLL("libtcamgststatistics.so")

# declare input/output type for our helper function
clib.tcam_statistics_get_structure.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_size_t]
clib.tcam_statistics_get_structure.restype = ctypes.c_bool

# allocate a c-string with length 320 bytes
# this _should_ be long enough to hold any
# GstStructure string that will be retrieved
# if not change this to a higher number
meta_out_buffer_size = 320
meta_out_buffer = ctypes.create_string_buffer(meta_out_buffer_size)


def get_meta(gst_buffer):
    """
    Check Gst.Buffer for a valid Gst.Meta object
    and return the contained Gst.Structure

    Parameters:
    Gst.Buffer

    Returns:
    Gst.Structure or None in case of error
    """
    meta = gst_buffer.get_meta("TcamStatisticsMetaApi")

    if not meta:
        return None

    # fill meta_out_buffer with the result of gst_structure_get_string(meta->structure)
    ret = clib.tcam_statistics_get_structure(hash(meta), meta_out_buffer, meta_out_buffer_size)

    # ret is a bool indicating successfull string copy
    if ret:
        # ret is bytecode
        # decode it to get a valide str object
        # ret.decode("utf-8")
        structure_string = ctypes.string_at(meta_out_buffer).decode("utf-8")
        struc = Gst.Structure.from_string(structure_string)
        # Gst.Structure.from_string returns a tuple
        # we only want the actual Gst.Structure
        return struc[0]
    return None


def callback(appsink, user_data):
    """
    This function will be called in a separate thread when our appsink
    says there is data for us. user_data has to be defined
    when calling g_signal_connect. It can be used to pass objects etc.
    from your other function to the callback.
    """
    sample = appsink.emit("pull-sample")

    if sample:
        print("new sample")
        # caps = sample.get_caps()

        gst_buffer = sample.get_buffer()

        tcam_meta = get_meta(gst_buffer)
        if tcam_meta:

            def print_structure(field_id, value, user_data):
                """
                """

                name = GLib.quark_to_string(field_id)

                print(f"{name} => {value}")

                # return true as we want to continue iterating
                return True

            # call print structure for all members of the Gst.Structure
            tcam_meta.foreach(print_structure, None)

        else:
            print("No meta")

    # empty line for more readability
    print("")

    return Gst.FlowReturn.OK


def main():

    Gst.init(sys.argv)
    serial = None

    pipeline = Gst.parse_launch("tcambin name=source"
                                " ! appsink name=sink")

    # test for error
    if not pipeline:
        print("Could not create pipeline.")
        sys.exit(1)

    # The user has not given a serial, so we prompt for one
    if serial is not None:
        source = pipeline.get_by_name("source")
        source.set_property("serial", serial)

    sink = pipeline.get_by_name("sink")

    # tell appsink to notify us when it receives an image
    sink.set_property("emit-signals", True)

    user_data = "This is our user data"

    # tell appsink what function to call when it notifies us
    sink.connect("new-sample", callback, user_data)

    pipeline.set_state(Gst.State.PLAYING)

    print("Press Ctrl-C to stop.")

    # We wait with this thread until a
    # KeyboardInterrupt in the form of a Ctrl-C
    # arrives. This will cause the pipline
    # to be set to state NULL
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        pipeline.set_state(Gst.State.NULL)


if __name__ == "__main__":
    main()
