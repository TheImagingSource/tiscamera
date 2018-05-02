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
import logging

log = logging.getLogger(__file__)


class Settings(object):

    def __init__(self):

        self.gc = "General"  # general category name used in config file

        self._set_defaults()

    def _set_defaults(self):
        """
        Set all settings to their default values
        """
        self.settings_directory = "~/.config/"
        self.settings_file_name = "./tcam-capture.conf"
        self.save_location = "/tmp"
        self.image_type = "png"
        self.video_type = "mpeg2"
        self.show_device_dialog_on_startup = True
        self.reopen_device_on_startup = True
        self.set_properties_on_reopen = True
        self.logfile_location = None
        self.use_dutils = True

    def reset(self):
        """Set properties to their default values"""
        self._set_defaults()

    def get_settings_file(self,
                          directory: str=os.getenv("XDG_CONFIG_DIR",
                                                   os.path.expanduser("~/.config")),
                          filename: str="tcam-capture.conf"):
        return os.path.join(directory, filename)

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

        self.save_location = gen.get("save_location", self.save_location)
        self.image_type = gen.get("image_type", self.image_type)
        self.video_type = gen.get("video_type", self.video_type)
        self.show_device_dialog_on_startup = gen.getboolean("show_device_dialog_on_startup",
                                                            self.show_device_dialog_on_startup)
        self.reopen_device_on_startup = gen.getboolean("reopen_device_on_startup",
                                                       self.reopen_device_on_startup)
        self.set_properties_on_reopen = gen.getboolean("set_properties_on_reopen",
                                                       self.set_properties_on_reopen)
        self.logfile_location = gen.get("log_file_location",
                                        self.logfile_location)
        self.use_dutils = gen.getboolean("use_dutils",
                                         self.use_dutils)
        return True

    def save(self):
        config = ConfigParser(allow_no_value=True)
        config.add_section(self.gc)

        config.set(self.gc, "# Location where images and videos shall be saved")
        config[self.gc]["save_location"] = self.save_location

        config.set(self.gc, "# file type used for saving images")
        config[self.gc]["image_type"] = self.image_type
        config.set(self.gc, "# file type used for saving videos")
        config[self.gc]["video_type"] = self.video_type

        config.set(self.gc, "# display dialog. ignored when reopen_device_on_startup is True")
        config[self.gc]["show_device_dialog_on_startup"] = str(self.show_device_dialog_on_startup)
        config.set(self.gc, "# Automatically open the last used device with its last known settings")
        config[self.gc]["reopen_device_on_startup"] = str(self.reopen_device_on_startup)
        config.set(self.gc, "# set device properties to their last known values")
        config[self.gc]["set_properties_on_reopen"] = str(self.set_properties_on_reopen)
        config.set(self.gc, "# folder to which log files should be written")
        config[self.gc]["log_file_location"] = str(self.logfile_location)
        config.set(self.gc, "# Use tiscamera-dutils, if present:")
        config[self.gc]["use_dutils"] = str(self.use_dutils)

        if not os.path.exists(self.settings_directory):
            os.makedirs(self.settings_directory)

        with open(self.get_settings_file(), 'w') as configfile:
            config.write(configfile)
            log.info("Saved config file '{}'".format(self.get_settings_file()))
