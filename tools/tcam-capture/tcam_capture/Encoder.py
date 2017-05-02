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

from enum import Enum, unique

import gi

gi.require_version("Gst", "1.0")

from gi.repository import Gst


@unique
class EncoderType(Enum):
    unknown = 0
    image = 1
    video = 2


class Encoder(object):

    def __init__(self, name, module, encoder_type: EncoderType, ending: str):
        self.name = name
        self.module = module
        self.encoder_type = encoder_type
        self.file_ending = ending


def get_encoder_dict():
    encoder_dict = {}
    if Gst.ElementFactory.find("pngenc") is not None:
        encoder_dict["png"] = Encoder("png", "pngenc", EncoderType.image, "png")
    if Gst.ElementFactory.find("jpegenc") is not None:
        encoder_dict["jpeg"] = Encoder("jpeg", "jpegenc", EncoderType.image, "jpeg")
    if Gst.ElementFactory.find("pnmenc") is not None:
        encoder_dict["pnm"] = Encoder("pnm", "pnmenc", EncoderType.image, "pnm")
    if Gst.ElementFactory.find("x264enc") is not None:
        encoder_dict["h264"] = Encoder("h264", "x264enc", EncoderType.video, "mp4")
    if Gst.ElementFactory.find("mpegtsmux") is not None:
        encoder_dict["mpeg2"] = Encoder("mpeg", "mpegtsmux", EncoderType.video, "mpeg")
    if Gst.ElementFactory.find("avimux") is not None:
        encoder_dict["avi"] = Encoder("avi", "avimux", EncoderType.video, "avi")

    return encoder_dict
