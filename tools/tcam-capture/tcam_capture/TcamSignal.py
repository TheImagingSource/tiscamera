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

from PyQt5.QtCore import QObject, pyqtSignal
import gi

gi.require_version("Gst", "1.0")

from gi.repository import Gst


class TcamSignals(QObject):

    change_property = pyqtSignal(object, str, object, str)
    update_device_list = pyqtSignal(list)
    start_stream = pyqtSignal(object)
    pause_stream = pyqtSignal(Gst.Element)
    stop_stream = pyqtSignal(object)
