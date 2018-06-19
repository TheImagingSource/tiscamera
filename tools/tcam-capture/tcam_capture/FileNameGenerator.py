
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

from tcam_capture.Settings import FileNameSettings
import datetime
import re
import os
import gi
gi.require_version("Gst", "1.0")

from gi.repository import Gst

import logging

log = logging.getLogger(__name__)


class FileNameGenerator(object):
    """"""

    def __init__(self,
                 serial: str,
                 settings: FileNameSettings,
                 caps: Gst.Caps=None):

        self.serial = serial
        self.settings = settings
        self.location = "/tmp/"
        self.file_suffix = "unknown"

        self.caps_str = FileNameGenerator.caps_to_fmt_string(caps)

    def set_settings(self, settings: FileNameSettings):
        """"""
        log.info("Received updated settings")
        self.settings = settings

    def set_serial(self, serial: str):
        """"""
        self.serial = serial

    def set_caps(self, caps: Gst.Caps):
        self.caps_str = FileNameGenerator.caps_to_fmt_string(caps)

    def _create_file_name_str(self,
                              fallthrough: str="mediafile",
                              create_searchpattern: bool=False):

        filename = ""

        if self.settings.user_prefix != "":
            filename += self.settings.user_prefix

        if self.settings.include_serial:
            if filename != "":
                filename += "-"
            filename += self.serial

        if self.settings.include_format:
            if filename != "":
                filename += "-"
            filename += self.caps_str

        if self.settings.include_timestamp:
            if filename != "":
                filename += "-"
            filename += datetime.datetime.now().strftime('%Y%m%dT%H%M%S')
        if self.settings.include_counter:
            if filename != "":
                filename += "-"

            if not create_searchpattern:
                # actual fillout
                next_index = FileNameGenerator.get_next_index(self.location,
                                                              self._create_file_name_str(fallthrough, True),
                                                              self.settings.counter_size)

                filename += '{message:0>{fill}}'.format(message=next_index,
                                                        fill=self.settings.counter_size)

            else:
                # use regex pattern to find preexisting files
                filename += "[0-9]{{{}}}"

        if filename == "":
            filename = fallthrough

        filename += "."
        filename += self.file_suffix

        log.debug("Returning filename {}".format(filename))

        return filename

    def create_file_name(self, fallthrough: str="mediafile"):
        """

        """

        fs = self.location + "/" + self._create_file_name_str(fallthrough)

        return fs

    @staticmethod
    def caps_to_fmt_string(caps: Gst.Caps):
        """
        Convert Gst.Caps to a string that is useable in file names.
        """

        if not caps or not caps.is_fixed():
            return ""

        fmt = caps.get_structure(0)
        width = fmt.get_value("width")
        height = fmt.get_value("height")
        frmt = fmt.get_value("format")

        fps_str = re.search("\d{1,2}/\d", fmt.to_string())

        fps_str = fps_str.group(0)

        return "{}_{}x{}_{}_{}".format(str(frmt),
                                       str(width),
                                       str(height),
                                       fps_str[:fps_str.find("/")],
                                       fps_str[fps_str.find("/")+1:])

    @staticmethod
    def get_next_index(directory: str,
                       filepattern: str,
                       index_width: int):
        """"""

        # python str.format uses {} to escape {}
        search_str = filepattern.format(index_width)

        matches = [f for f in os.listdir(directory) if re.search(search_str, f)]

        if not matches:
            return 0

        numbers = []

        for m in matches:

            # search for a number that has our width and is followed by a '.'
            # the structure of our filenames is static,
            # thus the index will always be last
            index_reg = r"\d{{{}}}\.".format(index_width)
            rem = re.search(index_reg, m)
            # remove the '.' before converting to int
            numbers.append(int(rem[0][:-1]))

        # overflow check
        # we need to check if the given padding is to small
        # e.g. index_width=5 what if 99999 exists and (maybe) 100000 or larger

        if (10 * index_width - 1) in numbers:
            ret = FileNameGenerator.get_next_index(directory,
                                                   filepattern,
                                                   index_width + 1)

            if ret != 0:
                return ret
            # else fall through and return value that
            # exceeds padding e.g. 99999 + 1

        value = max(numbers) + 1

        return value
