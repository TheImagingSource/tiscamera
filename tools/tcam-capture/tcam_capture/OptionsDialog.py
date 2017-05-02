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

from . import Encoder
from . import Settings
# from PyQt5.QtGui import Q
from PyQt5.QtWidgets import (QDialog, QGridLayout, QVBoxLayout,
                             QLabel, QLineEdit, QComboBox,
                             QDialogButtonBox, QFileDialog,
                             QPushButton, QHBoxLayout)
from PyQt5.QtCore import Qt


class OptionsDialog(QDialog):
    def __init__(self, parent=None):
        super(OptionsDialog, self).__init__(parent)

        encoder_dict = Encoder.get_encoder_dict()

        self.setWindowTitle("Tcam-Capture Options")
        self.layout = QVBoxLayout(self)

        location_layout = QHBoxLayout()
        self.location_label = QLabel("Where to save images/videos:", self)
        self.location_edit = QLineEdit(self)
        self.location_dialog_button = QPushButton("...", self)
        self.location_dialog_button.clicked.connect(self.open_file_dialog)
        location_layout.addWidget(self.location_label)
        location_layout.addWidget(self.location_edit)
        location_layout.addWidget(self.location_dialog_button)

        self.layout.addItem(location_layout)

        image_layout = QHBoxLayout()
        self.image_type_label = QLabel("Save images as:", self)
        self.image_type_combobox = QComboBox(self)
        for key, value in encoder_dict.items():
            if value.encoder_type == Encoder.EncoderType.image:
                self.image_type_combobox.addItem(key)

        image_layout.addWidget(self.image_type_label)
        image_layout.addWidget(self.image_type_combobox)

        self.layout.addItem(image_layout)

        video_layout = QHBoxLayout()
        self.video_type_label = QLabel("Save videos as:", self)
        self.video_type_combobox = QComboBox(self)
        for key, value in encoder_dict.items():
            if value.encoder_type == Encoder.EncoderType.video:
                self.video_type_combobox.addItem(key)

        video_layout.addWidget(self.video_type_label)
        video_layout.addWidget(self.video_type_combobox)

        self.layout.addItem(video_layout)

        # OK and Cancel buttons
        self.buttons = QDialogButtonBox(
            QDialogButtonBox.Ok | QDialogButtonBox.Cancel,
            Qt.Horizontal, self)
        self.layout.addWidget(self.buttons)

        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)

        self.setLayout(self.layout)

    def set_settings(self, settings: Settings):
        self.location_edit.setText(settings.get_save_location())
        self.image_type_combobox.setCurrentText(settings.get_image_type())
        self.video_type_combobox.setCurrentText(settings.get_video_type())

    def open_file_dialog(self):
        fdia = QFileDialog()
        fdia.setFileMode(QFileDialog.Directory)
        fdia.setWindowTitle("Select Directory for saving images and videos")
        if fdia.exec_():
            self.location_edit.setText(fdia.selectedFiles()[0])

    def get_location(self):
        return self.location_edit.text()

    def get_image_format(self):
        return self.image_type_combobox.currentText()

    def get_video_format(self):
        return self.video_type_combobox.currentText()

    @staticmethod
    def get_options(settings=None, parent=None):
        dialog = OptionsDialog(parent)

        if settings is not None:
            dialog.set_settings(settings)
        result = dialog.exec_()

        location = dialog.get_location()
        image = dialog.get_image_format()
        video = dialog.get_video_format()

        return (result == QDialog.Accepted, location, image, video)
