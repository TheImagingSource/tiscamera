#
# Copyright 2013-2014 The Imaging Source Europe GmbH
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

import gst
import glib
import gobject

WIDTH = 744
HEIGHT = 480

gobject.threads_init()


def on_new_buffer(sink, buffer, pad, src):
    structure = buffer.get_caps().get_structure(0)
    if buffer.size != (structure["width"] * structure["height"]):
        return
    src.emit("push-buffer", buffer)

video_pipeline = gst.parse_launch('v4l2src device=/dev/video1 '
                                  '! video/x-raw-gray,width=%d,height=%d '
                                  '! fakesink signal-handoffs=true name="sink"'
                                  % (WIDTH, HEIGHT))
display_pipeline = gst.parse_launch('appsrc name="src" '
                                    '! queue '
                                    '! ffmpegcolorspace '
                                    '! ximagesink sync=false')

sink = video_pipeline.get_by_name("sink")
src = display_pipeline.get_by_name("src")

sink.connect("handoff", on_new_buffer, src)

print ("Starting pipelines...")
display_pipeline.set_state(gst.STATE_PLAYING)
video_pipeline.set_state(gst.STATE_PLAYING)
print ("Done")

loop = glib.MainLoop()

loop.run()
