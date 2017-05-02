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
import logging
import gi

gi.require_version("Gst", "1.0")

from gi.repository import Gst

log = logging.getLogger(__name__)


class ImageSaver(object):
    """
    Allows the saving of an image from a pipeline.
    Encoding is chosen by the user.
    Default location is /tmp
    """
    def __init__(self, pipeline, serial):
        self.pipeline = pipeline
        self.serial = serial
        self.index = 0
        self.location = "/tmp/"

        self.encoder_dict = get_encoder_dict()
        self.selected_image_encoder = self.encoder_dict["png"]
        self.recording_type = EncoderType.unknown

    def set_image_encoder(self, enc_str: str):
        if not any(enc_str == y.name for x, y in self.encoder_dict.items()):
            return False

        if self.encoder_dict[enc_str].encoder_type is not EncoderType.image:
            return False

        self.selected_image_encoder = self.encoder_dict[enc_str]
        return True

    def set_location(self, location):
        if os.path.exists(location) and os.path.isdir(location):
            self.location = location
            return True
        return False

    def _bus_call(self, gst_bus, message):
        """"""
        t = message.type
        if (t == Gst.MessageType.ELEMENT):
            if (message.get_structure().has_name("GstMultiFileSink")):
                self.index = self.index + 1
                self._disconnect()

    def _create_encoder_element(self, enc_type: EncoderType):
        log.debug("Using encoder: {}".format(self.selected_video_encoder.module))
        return Gst.ElementFactory.make(self.selected_image_encoder.module,
                                       "encoder")

    def generate_location(self):
        return (self.location + "/"
                + self.serial + "."
                + self.selected_image_encoder.name)

    def create_pipeline(self, enc_type: EncoderType):
        self.image_bin = Gst.Bin.new()

        self.image_queue = Gst.ElementFactory.make("queue", "image_queue")
        self.convert = Gst.ElementFactory.make("videoconvert", "image_convert")
        self.encoder = self._create_encoder_element(enc_type)
        self.filesink = Gst.ElementFactory.make("multifilesink", "image_sink")
        self.filesink.set_property("index", self.index)
        self.filesink.set_property("post-messages", True)

        location = self.generate_location()
        log.info("Saving to: {}".format(location))

        self.filesink.set_property("location", location)

        self.image_bin.add(self.image_queue)
        self.image_bin.add(self.convert)
        self.image_bin.add(self.encoder)
        self.image_bin.add(self.filesink)

        self.image_queue.link(self.convert)
        self.convert.link(self.encoder)
        self.encoder.link(self.filesink)

        queue_pad = self.image_queue.get_static_pad("sink")

        self.ghost = Gst.GhostPad.new("bin_sink", queue_pad)
        self.image_bin.add_pad(self.ghost)

    def get_selected_image_encoder_type(self):
        return self.selected_image_encoder.encoder_type

    def save_image(self, image_type: str):
        if not self.set_image_encoder(image_type):
            return False

        self.create_pipeline(EncoderType.image)
        self.recording_type = EncoderType.image
        return self._connect()

    def _connect(self):
        self.pipeline.set_state(Gst.State.PAUSED)

        tee = self.pipeline.get_by_name("tee")
        if tee is None:
            log.error("Could not find tee.")
            return False
        srcpad = tee.get_request_pad("src_%u")
        self.pipeline.add(self.image_bin)

        try:
            srcpad.link(self.ghost)
        except Gst.LinkError as e:
            log.error("Could not link image saving bin to pipeline")
            return

        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        self.watch_id = bus.connect("message::element",
                                    self._bus_call)
        self.pipeline.set_state(Gst.State.PLAYING)
        log.debug("Attached image saving bin")
        return True

    def _disconnect(self):

        self.pipeline.set_state(Gst.State.PAUSED)

        tee = self.pipeline.get_by_name("tee")

        if tee is None:
            log.error("Could not find tee.")
            return

        srcpad = tee.get_request_pad("src_%u")
        self.pipeline.remove(self.image_bin)
        srcpad.unlink(self.ghost)

        bus = self.pipeline.get_bus()
        # bus.remove_signal_watch()
        bus.disconnect(self.watch_id)
        self.pipeline.set_state(Gst.State.PLAYING)
        log.debug("Disconnected image saving bin")
