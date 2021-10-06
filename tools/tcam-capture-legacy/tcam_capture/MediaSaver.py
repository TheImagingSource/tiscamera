# Copyright 2018 The Imaging Source Europe GmbH
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


from .Encoder import MediaType, get_encoder_dict, Encoder

from PyQt5.QtCore import pyqtSignal, QObject
import os
import gi

gi.require_version("Gst", "1.0")

from gi.repository import Gst

import logging

log = logging.getLogger(__name__)


class MediaSaver(QObject):
    """"""

    saved = pyqtSignal(str)
    error = pyqtSignal(str)

    def __init__(self, serial,
                 caps: Gst.Caps,
                 media_type: MediaType):
        """"""
        super(MediaSaver, self).__init__()
        self.pipeline = None
        self.src = None
        self.serial = serial
        self.index = 0
        self.location = "/tmp/"
        self.media_type = media_type
        self.encoder_dict = get_encoder_dict()
        self.video_encoder = self.encoder_dict["avi"]
        self.image_encoder = self.encoder_dict["png"]
        self.caps = caps
        self.accept_buffer = False

        self.current_filename = ""
        self.working = False
        self.queue_counter = 0
        self.src_name = ""
        self.sink_name = ""
        self.pipeline_name = ""
        self.__find_element_names()

    def __del__(self):
        """
        This destructor exists to ensure a graceful exit of all gstreamer pipelines
        """
        if self.pipeline:
            self.pipeline.set_state(Gst.State.NULL)
            self.src = None
            self.pipeline = None

    def __find_element_names(self):
        """
        Create strings describing the element names for out instance
        """

        if self.media_type == MediaType.image:
            self.src_name = "mediasaver-image-src"
            self.sink_name = "mediasaver-image-sink"
            self.pipeline_name = "mediasaver-image-pipeline"
        else:
            self.src_name = "mediasaver-video-src"
            self.sink_name = "mediasaver-video-sink"
            self.pipeline_name = "mediasaver-video-pipeline"

    def set_encoder(self, enc_str: str):

        if self.encoder_dict[enc_str].encoder_type is not MediaType.video:
            return False

        self.selected_video_encoder = self.encoder_dict[enc_str]
        return True

    def set_caps(self, caps: Gst.Caps):
        self.caps = caps

    def _generate_location(self):
        """
        Generate the location string that is used by either
        filesink or multifilesink for saving files


        """
        return self.location + "/" + "mediafile." + self._select_encoder().file_ending

    def _select_encoder(self):
        """"""
        if self.media_type == MediaType.image:
            return self.image_encoder
        return self.video_encoder

    def _select_sink(self):
        """
        Select the appropriate gstreamer sink
        """

        if self.media_type == MediaType.image:
            return "multifilesink post-messages=true"
        return "filesink"

    def _create_pipeline(self):
        """
        Create a GstPipeline that contains our encoding, etc
        """
        if self.current_filename == "":
            location = self._generate_location()
        else:
            location = self.current_filename

        encoder = self._select_encoder()
        sink_str = self._select_sink()

        save_str = ("appsrc name={} is-live=true format=3 "
                    "! {} "
                    "! queue leaky=downstream "
                    "! videoconvert "
                    "! {} "
                    "! {} name={} location={}").format(self.src_name,
                                                       self.caps.to_string(),
                                                       encoder.module,
                                                       sink_str,
                                                       self.sink_name,
                                                       location)

        log.info("Using pipeline to save: '{}'".format(save_str))

        self.pipeline = Gst.parse_launch(save_str)

        self.src = self.pipeline.get_by_name(self.src_name)
        self.src.set_property("caps", self.caps)
        self.src.set_property("do-timestamp", True)

        self.pipeline.set_name(self.pipeline_name)

    def _bus_call(self, gst_bus, message):
        """

        """
        t = message.type

        # log.info("Received msg from {}".format(message.src.get_name()))
        if message.src.get_name() == self.sink_name:

            # log.info("{}".format(message.get_structure().to_string()))
            # log.info("{}".format(message.get_structure().get_string("filename")))
            self.saved.emit(message.get_structure().get_string("filename"))
            self.current_filename = ""

        if t == Gst.MessageType.EOS:
            # log.info("Received EOS from {}".format(message.src.get_name()))

            if (message.src.get_name() == self.sink_name and
                    self.media_type == MediaType.video):
                self.saved.emit(self.current_filename)
                # log.info("sink sent EOS {}".format(message.get_structure().to_string()))
                self.current_filename = ""

            self.pipeline.set_state(Gst.State.NULL)
            self.src = None
            self.pipeline = None
            self.working = False
            self.saved.emit(self.current_filename)

    def feed_image(self, gstbuffer: Gst.Buffer):
        """
        Feed gstbuffer into the pipeline
        """
        if not self.working or not self.accept_buffer:
            return
        if self.src:
            self.src.emit("push-buffer", gstbuffer)

        if self.media_type == MediaType.image:
            log.debug("pushing buffer")
            self.queue_counter -= 1
            if self.queue_counter == 0:
                self.accept_buffer = False

    def start_recording_video(self, encoder):
        """
        Start saving a video
        """
        if self.working:
            return
        self.video_encoder = get_encoder_dict()[encoder]
        if self.media_type != MediaType.video:
            log.error("Requested encoder is not intended for videos. Aborting.")
            return
        self._create_pipeline()

        self.pipeline.set_state(Gst.State.PLAYING)
        self.working = True
        self.accept_buffer = True
        log.info("Saving to....{}".format(self._generate_location()))
        self.index += 1

        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect("message", self._bus_call)

    def stop_recording_video(self):
        """
        Stop the video pipeline
        """

        if self.media_type != MediaType.video:
            return

        self.accept_buffer = False
        self.pipeline.send_event(Gst.Event.new_eos())
        log.debug("Sent EOS to saving pipeline")

    def save_image(self, encoder: Encoder):
        """
        Trigger the saving of a single image
        """
        if self.media_type != MediaType.image:
            return

        if encoder.encoder_type != MediaType.image:
            log.error("Specified encoder can not be used for images. Aborting.")
            return

        self.image_encoder = encoder

        if not self.pipeline:
            self._create_pipeline()
            self.pipeline.set_state(Gst.State.PLAYING)

            bus = self.pipeline.get_bus()
            bus.add_signal_watch()
            bus.connect("message", self._bus_call)

        sink = self.pipeline.get_by_name(self.sink_name)
        sink.set_property("location", self.current_filename)

        self.working = True
        self.accept_buffer = True
        self.queue_counter += 1
