
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

import glob
import datetime
import re
import os
import gi
gi.require_version("Gst", "1.0")

from gi.repository import Gst


import logging

log = logging.getLogger(__name__)


class FileNamePattern(object):
    """"""

    def __init__(self,
                 name: str, pattern: str, help_text: str, sample: str):
        self.name = name
        self.pattern = pattern
        self.help_text = help_text
        self.sample = sample


class FileNameGenerator(object):
    """"""

    def __init__(self):

        # self.pattern_default = "{serial}-{format}-%Y%m%dT%H%M%S-{counter:5}.{suffix}"
        self.pattern_default = "{serial}-{format}-{counter:5}.{suffix}"
        self.pattern = self.pattern_default

        self.fields = {}
        self.fields["serial"] = FileNamePattern("serial",
                                                "{serial}",
                                                "serial number of the device that is used.",
                                                "00001234")

        self.fields["suffix"] = FileNamePattern("suffix",
                                                "{suffix}",
                                                "Automatically determine the file format and set the file type accordingly.",
                                                "mkv")

        self.fields["format"] = FileNamePattern("format",
                                                "{format}",
                                                "Simple format description of what the device currently sends.",
                                                "jpeg_1920x1080_60_1")

        self.fields["counter"] = FileNamePattern("counter",
                                                 "{counter(\:\d+)?}",
                                                 "Simple counter that is increased with every image taken. Can be padded by appending :<int>",
                                                 "00001")

    def get_pattern_list(self):
        pass

    def _replace(self, text, rep_dict):
        """"""

        # use these three lines to do the replacement
        rep_dict = dict((re.escape(k), v) for k, v in rep_dict.items())
        pattern = re.compile("|".join(rep_dict.keys()))
        text = pattern.sub(lambda m: rep_dict[re.escape(m.group(0))], text)

        def replace_counter(match):
            if match.group() == "{counter}":
                return rep_dict["{counter}"]
            else:
                padding = re.search("\d+", match.group())
                return "%{}d".format(padding.group())
                return "{counter:0{width}d}".format(width=padding.group(),
                                                    counter=1)

        # deal with counter separately as its padding makes simple replacement impossible
        # text = re.sub(r'{counter(\:\d+)?}', replace_counter, text)
        text = re.sub(r'{counter(\:\d+)?}', "%05d", text)

        text = datetime.datetime.strftime(datetime.datetime.now(), text)
        log.info("returning {}".format(text))
        return text

    def set_pattern(self, pattern: str = None):
        """
        Sets the useed pattern to the specified string
        Should the given pattern be None, the pattern will be reset
        by calling reset_pattern
        """
        return
        if pattern:
            self.pattern = pattern
        else:
            self.reset_pattern()

    def reset_pattern(self):
        """Resets pattern to the default value"""
        self.pattern = self.pattern_default

    def _create_replacement_dict(self,
                                 serial: str=None,
                                 fmt: str=None,
                                 counter: str="",
                                 file_suffix: str=None):
        rep_dict = {}
        rep_dict["{format}"] = fmt
        rep_dict["{suffix}"] = file_suffix
        rep_dict["{counter}"] = counter
        rep_dict["{serial}"] = serial

        return rep_dict

    def preview(self, pattern: str):
        """
        Return a preview of how a filename with
        the specified pattern would look like
        """
        #  TODO replace stuff with things from filenamepatterns
        rep_dict = self._create_replacement_dict("00001234",
                                                 "jpeg_1920x1080_60_1",
                                                 "1",
                                                 "pnm")
        return self._replace(pattern, rep_dict)

    def create_file_name(self,
                         serial: str=None,
                         fmt: str=None,
                         counter: str=None,
                         file_suffix: str=None):

        rep_dict = self._create_replacement_dict(serial,
                                                 fmt,
                                                 "%05d",
                                                 file_suffix)

        return self._replace(self.pattern, rep_dict)

    @staticmethod
    def caps_to_fmt_string(caps: Gst.Caps):
        """"""
        if not caps.is_fixed():
            return ""

        fmt = caps.get_structure(0)
        width = fmt.get_value("width")
        height = fmt.get_value("height")
        frmt = fmt.get_value("format")

        fps_str = re.search("\d{1,2}/\d", fmt.to_string())

        fps_str = fps_str.group(0)
        # fmt.to_string()[fmt.to_string().find("framerate=(fraction)"):]

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
        search_str = "19510261-[0-9]{{{}}}.jpeg".format(index_width)

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
            ret = FileNameGenerator.get_next_index(index_width + 1)

            if ret != 0:
                return ret
            # else fall through and return value that
            # exceeds padding e.g. 99999 + 1

        value = max(numbers) + 1

        return value
