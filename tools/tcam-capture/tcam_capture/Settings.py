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


class FileNameSettings():

    def __init__(self):

        self.include_timestamp = True
        self.include_serial = True
        self.include_counter = True
        self.overwrite_files = False
        self.counter_size = 5
        self.include_format = True
        self.user_prefix = ""


class Settings(object):

    def __init__(self):

        self.section_general = "General"  # general category name used in config file
        self.section_image_name = "Image Naming"
        self.section_video_name = "Video Naming"
        self._set_defaults()

    def _set_defaults(self):
        """
        Set all settings to their default values
        """
        self.settings_directory = "~/.config/"
        self.settings_file_name = "./tcam-capture.conf"
        self.save_location = "/tmp"
        self.image_type = "png"
        self.video_type = "avi"
        self.show_device_dialog_on_startup = True
        self.reopen_device_on_startup = True
        self.set_properties_on_reopen = True
        self.logfile_location = None
        self.use_dutils = True

        self.image_name = FileNameSettings()
        self.video_name = FileNameSettings()

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
        sg = self.section_general

        gen = config[sg]

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

        if config.has_section(self.section_image_name):

            img = config[self.section_image_name]
            self.image_name.counter_size = img.getint("counter_size",
                                                      self.image_name.counter_size)
            self.image_name.include_counter = img.getboolean("counter",
                                                             self.image_name.include_counter)
            self.image_name.overwrite_files = img.getboolean("overwrite_files",
                                                             self.image_name.overwrite_files)
            self.image_name.include_serial = img.getboolean("serial",
                                                            self.image_name.include_serial)
            self.image_name.include_format = img.getboolean("format",
                                                            self.image_name.include_format)
            self.image_name.include_timestamp = img.getboolean("timestamp",
                                                               self.image_name.include_timestamp)
            self.image_name.user_prefix = img.get("user-prefix",
                                                  self.image_name.user_prefix)

        if config.has_section(self.section_video_name):

            img = config[self.section_video_name]
            self.video_name.counter_size = img.getint("counter_size",
                                                      self.video_name.counter_size)
            self.video_name.include_counter = img.getboolean("counter",
                                                             self.video_name.include_counter)
            self.video_name.overwrite_files = img.getboolean("overwrite_files",
                                                             self.video_name.overwrite_files)
            self.video_name.include_serial = img.getboolean("serial",
                                                            self.video_name.include_serial)
            self.video_name.include_format = img.getboolean("format",
                                                            self.video_name.include_format)
            self.video_name.include_timestamp = img.getboolean("timestamp",
                                                               self.video_name.include_timestamp)
            self.video_name.user_prefix = img.get("user-prefix",
                                                  self.video_name.user_prefix)
        return True

    def save(self):

        sg = self.section_general

        config = ConfigParser(allow_no_value=True)
        config.add_section(sg)

        config.set(sg, "# Location where images and videos shall be saved")
        config[sg]["save_location"] = self.save_location

        config.set(sg, "# file type used for saving images")
        config[sg]["image_type"] = self.image_type
        config.set(sg, "# file type used for saving videos")
        config[sg]["video_type"] = self.video_type

        config.set(sg, "# display dialog. ignored when reopen_device_on_startup is True")
        config[sg]["show_device_dialog_on_startup"] = str(self.show_device_dialog_on_startup)
        config.set(sg, "# Automatically open the last used device with its last known settings")
        config[sg]["reopen_device_on_startup"] = str(self.reopen_device_on_startup)
        config.set(sg, "# set device properties to their last known values")
        config[sg]["set_properties_on_reopen"] = str(self.set_properties_on_reopen)
        config.set(sg, "# folder to which log files should be written")
        config[sg]["log_file_location"] = str(self.logfile_location)
        config.set(sg, "# Use tiscamera-dutils, if present:")
        config[sg]["use_dutils"] = str(self.use_dutils)

        img = self.section_image_name
        config.add_section(img)
        config.set(img, "# user defined prefix")
        config[img]["user-prefix"] = str(self.image_name.user_prefix)
        config.set(img, "# include serial in filename")
        config[img]["serial"] = str(self.image_name.include_serial)
        config.set(img, "# include current format in filename")
        config[img]["format"] = str(self.image_name.include_format)
        config.set(img, "# include current ISO timestamp in filename ")
        config[img]["timestamp"] = str(self.image_name.include_timestamp)
        config.set(img, "# include a counter in filename")
        config[img]["counter"] = str(self.image_name.include_counter)
        config.set(img, "# minimum size of the counter (padding)")
        config[img]["counter_size"] = str(self.image_name.counter_size)
        config.set(img, "# overwrite files or try to always use unique names")
        config[img]["overwrite_files"] = str(self.image_name.overwrite_files)

        vid = self.section_video_name
        config.add_section(vid)
        config.set(vid, "# user defined prefix")
        config[vid]["user-prefix"] = str(self.video_name.user_prefix)
        config.set(vid, "# include serial in filename")
        config[vid]["serial"] = str(self.video_name.include_serial)
        config.set(vid, "# include current format in filename")
        config[vid]["format"] = str(self.video_name.include_format)
        config.set(vid, "# include current ISO timestamp in filename ")
        config[vid]["timestamp"] = str(self.video_name.include_timestamp)
        config.set(vid, "# include a counter in filename")
        config[vid]["counter"] = str(self.video_name.include_counter)
        config.set(vid, "# minimum size of the counter (padding)")
        config[vid]["counter_size"] = str(self.video_name.counter_size)
        config.set(vid, "# overwrite files or try to always use unique names")
        config[vid]["overwrite_files"] = str(self.video_name.overwrite_files)

        if not os.path.exists(os.path.expanduser(self.settings_directory)):
            os.makedirs(os.path.expanduser(self.settings_directory))

        with open(self.get_settings_file(), 'w') as configfile:
            config.write(configfile)
            log.info("Saved config file '{}'".format(self.get_settings_file()))
