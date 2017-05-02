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


from .Encoder import EncoderType, get_encoder_dict

import os
import gi

gi.require_version("Gst", "1.0")

from gi.repository import Gst

import logging

log = logging.getLogger(__name__)


class VideoSaver(object):
    """"""
    def __init__(self, pipeline, serial):
        self.pipeline = pipeline
        self.serial = serial
        self.index = 0
        self.location = "/tmp/"
        self.running = False
        self.encoder_dict = get_encoder_dict()
        self.selected_video_encoder = self.encoder_dict["mpeg2"]
        self.queue_pad = None
        self.watch_id = None

    def set_video_encoder(self, enc_str: str):

        if self.encoder_dict[enc_str].encoder_type is not EncoderType.video:
            return False

        self.selected_video_encoder = self.encoder_dict[enc_str]
        return True

    def set_location(self, location):
        if os.path.exists(location) and os.path.isdir(location):
            self.location = location
            return True
        return False

    def _create_encoder_element(self, enc_type: EncoderType):
        log.debug("Using encoder: {}".format(self.selected_video_encoder.module))
        element = Gst.ElementFactory.make(self.selected_video_encoder.module,
                                          "encoder")

        if self.selected_video_encoder.name == "h264":
            element.set_property("tune", "zerolatency")

        return element

    def generate_location(self):
        return (self.location + "/"
                + self.serial + "."
                + self.selected_video_encoder.file_ending)

    def create_pipeline(self, enc_type: EncoderType):
        self.image_bin = Gst.Bin.new()

        bin_name = "VideoSaveBin"
        self.image_bin.set_property("message-forward", True)
        self.image_bin.set_property("name", bin_name)

        self.image_queue = Gst.ElementFactory.make("queue", bin_name + "-queue")
        self.convert = Gst.ElementFactory.make("videoconvert", bin_name + "-convert")

        self.encoder = self._create_encoder_element(enc_type)

        self.mux = Gst.ElementFactory.make("mpegtsmux", bin_name + "-mux")

        self.filesink = Gst.ElementFactory.make("filesink", bin_name + "-sink")

        self.filesink.set_property("sync", False)

        location = self.generate_location()
        log.info("Saving to: {}".format(location))

        self.filesink.set_property("location", self.generate_location())
        self.image_bin.add(self.image_queue)
        self.image_bin.add(self.convert)
        self.image_bin.add(self.encoder)
        self.image_bin.add(self.mux)
        self.image_bin.add(self.filesink)

        self.image_queue.link(self.convert)
        self.convert.link(self.encoder)
        self.encoder.link(self.mux)
        self.mux.link(self.filesink)

        self.queue_pad = self.image_queue.get_static_pad("sink")
        self.ghost = Gst.GhostPad.new("bin_sink", self.queue_pad)
        self.image_bin.add_pad(self.ghost)

    def start_recording_video(self, video_type: str):
        if self.running is True:
            return False
        if not self.set_video_encoder(video_type):
            log.error("Unable to set video encoder")
            return False
        self.create_pipeline(EncoderType.video)
        if self._connect():
            self.running = True
            return True
        return False

    def stop_recording_video(self):
        if self.running:
            return self._disconnect()
        else:
            return False

    def _connect(self):
        self.running = True
        self.pipeline.set_state(Gst.State.PAUSED)

        tee = self.pipeline.get_by_name("tee")
        if tee is None:
            log.error("Could not find tee.")
            return False
        self.srcpad = tee.get_request_pad("src_%u")
        self.pipeline.add(self.image_bin)

        try:
            self.srcpad.link(self.ghost)
        except Gst.LinkError as e:
            log.error("Could not link video save bin to pipeline.")
            self.pipeline.set_state(Gst.State.PLAYING)
            return False

        self.pipeline.set_state(Gst.State.PLAYING)

        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        self.watch_id = bus.connect("message::element",
                                    self._bus_call)
        self.watch_id = bus.connect("message::eos",
                                    self._bus_call)

        log.info("Linked video save bin to pipeline.")
        return True

    def _bus_call(self, gst_bus, message):
        """"""
        t = message.type
        if t == Gst.MessageType.ELEMENT:
            if message.get_structure().has_name("GstBinForwarded"):
                if message.get_structure().has_field("message"):
                    if not self.running:
                        self.pipeline.set_state(Gst.State.PAUSED)
                        self.pipeline.remove(self.image_bin)
                        self.image_bin.set_state(Gst.State.READY)
                        self.image_bin.set_state(Gst.State.NULL)
                        self.pipeline.set_state(Gst.State.PLAYING)
                        self.ghost = None

        return True

    def buffer_probe(self, pad, probe_info, user_data):

        if self.ghost is not None:
            if self.srcpad.unlink(self.ghost):
                log.debug("unlinked video save bin")
            else:
                log.error("could not unlink video save bin")

        return Gst.PadProbeReturn.REMOVE

    def _disconnect(self):
        log.debug("disconnecting")
        self.running = False
        if self.ghost is not None:
            self.ghost.send_event(Gst.Event.new_eos())
            self.probe_id = self.queue_pad.add_probe(Gst.PadProbeType.BLOCKING,
                                                     self.buffer_probe, None)
