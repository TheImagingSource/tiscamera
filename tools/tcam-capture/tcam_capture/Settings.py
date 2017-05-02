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

from configparser import ConfigParser
import os


class Settings(object):

    def __init__(self):

        self.gc = "General"  # general category name used in config file

        self.set_defaults()

    def set_defaults(self):
        self.settings_directory = "/home/edt/.config/"
        self.settings_file_name = "/tcam_capture.ini"
        self.save_location = "/tmp"
        self.image_type = "png"
        self.video_type = "mpeg2"

    def get_settings_file(self):
        return self.settings_directory + self.settings_file_name

    def get_save_location(self):
        return self.save_location

    def get_image_type(self):
        return self.image_type

    def get_video_type(self):
        return self.video_type

    def set_location(self, location):
        self.save_location = location

    def set_image_type(self, image):
        self.image_type = image

    def set_video_type(self, video):
        self.video_type = video

    def load(self):

        if not os.path.isfile(self.get_settings_file()):
            return False
        config = ConfigParser()
        config.read(self.get_settings_file())

        gen = config[self.gc]

        self.save_location = gen.get("SaveLocation", self.save_location)
        self.image_type = gen.get("ImageType", self.image_type)
        self.video_type = gen.get("VideoType", self.video_type)
        return True

    def save(self):
        config = ConfigParser()
        config.add_section(self.gc)
        config[self.gc]["SaveLocation"] = self.save_location
        config[self.gc]["ImageType"] = self.image_type
        config[self.gc]["VideoType"] = self.video_type

        if not os.path.exists(self.settings_directory):
            os.makedirs(self.settings_directory)

        with open(self.get_settings_file(), 'w') as configfile:
            config.write(configfile)
