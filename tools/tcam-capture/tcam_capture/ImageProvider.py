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


from . import image_dir
import os
from PyQt5.QtGui import QIcon


class ImageProvider():
    """"""
    def __init__(self):
        self.path = image_dir.tcam_image_location

    def _find_correct_image_location(self):
        pass

    def get_device_image(self, name: str = "default"):

        if name == "default":
            return self.get_default()

    def get_default(self):
        fullpath = os.path.join(self.path, "23uc256.png")
        if os.path.isfile(fullpath):
            return QIcon(fullpath)
        else:
            return QIcon.fromTheme('camera-photo')

    def get_tcam_logo(self):
        fullpath = os.path.join(self.path, "tiscamera_logo.png")
        if os.path.isfile(fullpath):
            return QIcon(fullpath)
        else:
            return QIcon.fromTheme('camera-photo')
