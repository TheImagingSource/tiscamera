#!/usr/bin/env python3

# Copyright 2022 The Imaging Source Europe GmbH
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
# This example will show you how to query the tcamsrc
# - to verify potential framerates
# - to verify potential caps
#

import sys
import gi
import time

gi.require_version("Gst", "1.0")

from gi.repository import  Gst

def main():

    Gst.init(sys.argv)

    pipeline = Gst.parse_launch("tcambin name=bin ! videoconvert ! ximagesink ")

    pipeline.set_state(Gst.State.READY)

    tcambin = pipeline.get_by_name("bin")

    # retrieve the source
    # these queries have to be performed on the source
    # as the tcambin might alter the query
    # which we do not want
    source = tcambin.get_by_name("tcambin-source")

    framerate_query_caps = Gst.Caps.from_string("video/x-bayer,format=rggb,width=640,height=480")
    # if you have a mono camera try these instead
    # framerate_query_caps = Gst.Caps.from_string("video/x-raw,format=GRAY8.width=640,height=480")

    framerate_query = Gst.Query.new_caps(framerate_query_caps)

    if source.query(framerate_query):

        # the result caps will contain our framerate_query_caps + all potential framerates
        supported_framerate_caps = framerate_query.parse_caps_result()
        print("Camera supports these framerates: {}".format(supported_framerate_caps.to_string()))

    #
    # verify specific caps
    # for this all needed fields have to be set (format, width, height and framerate)
    #

    accept_query_caps = Gst.Caps.from_string("video/x-bayer,format=rggb,width=640,height=480,framerate=30/1")

    accept_query = Gst.Query.new_accept_caps(accept_query_caps)

    if source.query(accept_query):

        if accept_query.parse_accept_caps_result():
            print("Caps are supported")
        else:
            print("Caps are not supported")

    pipeline.set_state(Gst.State.NULL)
    return 0


if __name__ == "__main__":
    sys.exit(main())
