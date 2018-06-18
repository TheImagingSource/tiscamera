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

from PyQt5.QtCore import pyqtSignal, QObject
from .Encoder import MediaType, get_encoder_dict
import os
import logging
import gi

gi.require_version("Gst", "1.0")

from gi.repository import Gst

log = logging.getLogger(__name__)


class ImageSaver(QObject):
    """
    Allows the saving of an image from a pipeline.
    Encoding is chosen by the user.
    Default location is /tmp
    """

    saved = pyqtSignal(str)
    error = pyqtSignal(str)

    def __init__(self, pipeline, serial):
        super(ImageSaver, self).__init__(None)
        self.pipeline = pipeline
        self.serial = serial
        self.index = 0
        self.location = "/tmp/"
        self.last_location = None
        self.encoder_dict = get_encoder_dict()
        self.selected_image_encoder = self.encoder_dict["png"]
        self.recording_type = MediaType.unknown
        self.srcpad = None
        self.working = False

    def _set_image_encoder(self, enc_str: str):
        """
        Selects the image encoder for later usage
        """
        if not any(enc_str == y.name for x, y in self.encoder_dict.items()):
            return False

        if self.encoder_dict[enc_str].encoder_type is not MediaType.image:
            return False

        self.selected_image_encoder = self.encoder_dict[enc_str]
        return True

    def _bus_call(self, gst_bus, message):
        """
        Callback from gstreamer that we can disconnect.
        """
        log.debug("got new message")
        t = message.type
        if (t == Gst.MessageType.ELEMENT):
            if (message.get_structure().has_name("GstMultiFileSink")):
                self.index = self.index + 1
                self.srcpad.add_probe(Gst.PadProbeType.IDLE,
                                      self._idle_probe,
                                      None)

    def _idle_probe(self, pad, probe, user_data):
        """
        Gstreamer probe to disconnect the bin
        """
        tee = self.pipeline.get_by_name("tee")

        if tee is None:
            log.error("Could not find tee.")
            return

        if self.srcpad:
            self.srcpad.unlink(self.ghost)
            tee.release_request_pad(self.srcpad)

        self.srcpad = None

        if self.image_bin:
            self.pipeline.remove(self.image_bin)

            self.image_bin.set_state(Gst.State.NULL)

        self.image_bin = None
        self.image_queue = None
        self.convert = None
        self.encoder = None
        self.filesink = None
        # log.debug("calling play")
        # self.pipeline.set_state(Gst.State.PLAYING)
        # log.debug("Disconnected image saving bin")

        bus = self.pipeline.get_bus()
        bus.disconnect(self.watch_id)
        self.working = False
        self.saved.emit(self.last_location)

        return Gst.PadProbeReturn.REMOVE

    def _create_encoder_element(self, enc_type: MediaType):
        """
        Convenience method to create gstreamer encoder elements
        """
        log.debug("Using encoder: {}".format(self.selected_image_encoder.module))
        return Gst.ElementFactory.make(self.selected_image_encoder.module,
                                       "encoder")

    def _generate_location(self):
        """
        Generate the absolute path to which the image shall be saved
        """
        return (self.location + "/" +
                self.serial + "-" + str(self.index) + "." +
                self.selected_image_encoder.name)

    def _create_pipeline(self, enc_type: MediaType):
        """
        Create the bin internal pipeline
        """
        self.image_bin = Gst.Bin.new()

        self.image_queue = Gst.ElementFactory.make("queue", "image_queue")
        self.convert = Gst.ElementFactory.make("videoconvert", "image_convert")
        self.encoder = self._create_encoder_element(enc_type)
        self.filesink = Gst.ElementFactory.make("multifilesink", "image_sink")
        self.filesink.set_property("index", self.index)
        self.filesink.set_property("post-messages", True)

        location = self._generate_location()
        log.info("Saving Image to: {}".format(location))
        self.last_location = location
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

    def save_image(self, image_type: str):
        """

        """
        if not self._set_image_encoder(image_type):
            return False
        self.working = True
        self._create_pipeline(MediaType.image)
        self.recording_type = MediaType.image
        return self._connect()

    def _connect(self):
        """
        Connect bin to actual pipeline
        """
        self.pipeline.set_state(Gst.State.PAUSED)

        tee = self.pipeline.get_by_name("tee")
        if tee is None:
            log.error("Could not find tee.")
            return False
        self.srcpad = tee.get_request_pad("src_%u")

        if not self.srcpad:
            log.error("Could not retrieve src pad to save image. Aborting.")
            self.pipeline.set_state(Gst.State.PLAYING)

            return

        self.pipeline.add(self.image_bin)

        try:
            self.srcpad.link(self.ghost)
        except Gst.LinkError as e:
            log.error("Could not link image saving bin to pipeline")
            return

        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.enable_sync_message_emission()

        self.watch_id = bus.connect("message::element",
                                    self._bus_call)
        self.pipeline.set_state(Gst.State.PLAYING)
        log.debug("Attached image saving bin")
        return True
