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

from .TcamDevice import TcamDevice, TcamDeviceIndex
from .ImageProvider import ImageProvider

from PyQt5.QtWidgets import (QDialog, QGridLayout, QVBoxLayout,
                             QLabel, QLineEdit, QComboBox,
                             QDialogButtonBox, QListWidget, QListWidgetItem,
                             QPushButton, QHBoxLayout, QWidget)
from PyQt5.QtGui import QPixmap, QIcon

from PyQt5.QtCore import Qt, QSize

import logging

log = logging.getLogger(__file__)


class SelectionWidget(QListWidgetItem):

    def __init__(self, device: TcamDevice, parent=None):
        super(SelectionWidget, self).__init__(parent)
        self.device = device
        self.image_provider = ImageProvider()
        self.setup_ui()

    def setup_ui(self):

        self.layout = QVBoxLayout()
        self.setTextAlignment(Qt.AlignHCenter)
        self.setText(self.device.model + "\n" + self.device.serial)
        self.select_icon()

    def select_icon(self):

        self.setIcon(self.image_provider.get_default())


class DeviceDialog(QDialog):

    def __init__(self, indexer: TcamDeviceIndex, parent=None):
        super(DeviceDialog, self).__init__(parent)

        self.indexer = indexer
        self.indexer.update_device_list.connect(self.update_device_list)

        self.setWindowTitle("Tcam-Capture Device Selection")
        self.setMinimumSize(640, 480)
        self.layout = QVBoxLayout(self)

        self.selected_item = None

        self.test_label = QLabel("Please select the device that shall be used:")

        self.layout.addWidget(self.test_label)

        self.camera_box = QListWidget(self)
        self.camera_box.setFlow(QListWidget.LeftToRight)
        self.camera_box.setGridSize(QSize(180, 180))
        self.camera_box.setViewMode(QListWidget.IconMode)
        self.camera_box.setResizeMode(QListWidget.Adjust)
        self.camera_box.setMovement(QListWidget.Static)
        self.camera_box.setWordWrap(True)
        self.camera_box.setAutoFillBackground(True)

        self.camera_box.setDragEnabled(False)
        self.camera_box.setTabKeyNavigation(True)

        self.camera_box.setIconSize(QSize(100, 100))

        # handle double click
        self.camera_box.itemActivated.connect(self.item_double_clicked)
        self.camera_box.itemDoubleClicked.connect(self.item_double_clicked)
        # handle a single click
        self.camera_box.itemChanged.connect(self.item_selected)
        self.camera_box.itemClicked.connect(self.item_selected)

        self.layout.addWidget(self.camera_box)

        # OK and Cancel buttons
        self.buttons = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel,
            Qt.Horizontal, self)
        self.layout.addWidget(self.buttons)

        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)

        self.setLayout(self.layout)

        # ensure box is populated when we start
        dev_list = self.indexer.device_list
        if dev_list:
            self.update_device_list(dev_list)

    def item_selected(self, item):
        """
        SLOT for self.camera_box.itemClicked and
        self.camera_box.itemChanged
        """
        self.selected_item = item

    def item_double_clicked(self, item):
        """SLOT for self.camera_box.itemDoubleClicked"""

        self.item_selected(item)

        self.accept()

    def get_selected_device(self):
        """
        Returns the curreclty selected TcamDevice may return None
        """

        if self.selected_item:
            return self.selected_item.device
        return None

    def update_device_list(self, device_list):
        """
        SLOT for indexer
        """

        # remember selected dev so that we can
        # keep the highlight when the list changes
        if self.selected_item:
            selected_dev = self.selected_item.device
        else:
            selected_dev = None

        self.camera_box.clear()

        entries = []
        for dev in device_list:
            item = SelectionWidget(dev)
            entries.append(item)
            self.camera_box.addItem(item)
            if selected_dev and dev.serial == selected_dev.serial:
                item.setSelected(True)

    @staticmethod
    def get_device(indexer, parent=None):
        dialog = DeviceDialog(indexer)

        result = dialog.exec_()

        dialog.hide()
        selected_device = dialog.get_selected_device()

        return (result == QDialog.Accepted, selected_device)
