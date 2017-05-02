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

from PyQt5 import QtCore
from PyQt5.QtCore import QObject, pyqtSignal

import gi

gi.require_version("Gst", "1.0")
gi.require_version("Tcam", "0.1")

from gi.repository import Tcam, Gst


class TcamDevice(object):
    """Simple representation of a Tcam Capture Device"""
    def __init__(self, device_type, serial, model):
        self.model = model
        self.device_type = device_type
        self.serial = serial

    def __repr__(self):
        return self.__str__()

    def __str__(self):
        return self.model + " - " + self.serial + " - " + self.device_type

    def __eq__(self, other):
        if (self.model == other.model and
            self.device_type == other.device_type and
            self.serial == other.serial):
            return True
        return False

    # required for determining if new elements are present in a
    # device list comparison
    def __lt__(self, other):
        return ("{}, {}, {}".format(self.serial,
                                    self.model,
                                    self.device_type)
                < "{}, {}, {}".format(other.serial,
                                      other.model,
                                      other.device_type))


class TcamDeviceIndex(QObject):
    """Index of available devices

    Updates periodically. Should be run in background thread.
    No start/stop needed. QTimer is killed automatically once
    the application quits."""

    update_device_list = pyqtSignal(list)

    def __init__(self):
        super().__init__()
        self.device_list = None
        self.sleep_period_seconds = 2
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.create_device_list)
        self.timer.start(self.sleep_period_seconds * 1000)

    def create_device_list(self):

        tcam = Gst.ElementFactory.make("tcamsrc")
        serials = tcam.get_device_serials()
        device_list = []

        for s in serials:
            (status, name, ident, connection_type) = tcam.get_device_info(s)
            d = TcamDevice(connection_type, s, name)
            device_list.append(d)

        # self.device_list will be None on first run
        if self.device_list is None or sorted(self.device_list) != sorted(device_list):
            self.update_device_list.emit(device_list)
            self.device_list = device_list
