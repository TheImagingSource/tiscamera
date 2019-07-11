# Copyright 2019 The Imaging Source Europe GmbH
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

from PyQt5.QtWidgets import QSlider
from PyQt5.QtCore import pyqtSignal, Qt


class TcamSlider(QSlider):
    """
    A default QSlider does not offer double click.
    We want this to allow certain action.
    """

    doubleClicked = pyqtSignal()

    def __init__(self, parent=None):
        super(TcamSlider, self).__init__(Qt.Horizontal, parent)

    def mouseDoubleClickEvent(self, event):

        self.doubleClicked.emit()
