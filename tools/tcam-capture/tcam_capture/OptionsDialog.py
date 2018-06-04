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

import logging

# from PyQt5.QtGui import Q
from PyQt5.QtWidgets import (QDialog, QGridLayout, QVBoxLayout,
                             QLabel, QLineEdit, QComboBox,
                             QDialogButtonBox, QFileDialog,
                             QPushButton, QHBoxLayout, QCheckBox,
                             QFormLayout)
from PyQt5.QtCore import Qt

log = logging.getLogger(__file__)


class OptionsDialog(QDialog):
    def __init__(self, setting: Settings, have_dutils, parent=None):
        super(OptionsDialog, self).__init__(parent)

        encoder_dict = Encoder.get_encoder_dict()
        self.settings = setting
        self.enabled_video = False
        self.enabled_dutils = have_dutils
        self.setWindowTitle("Tcam-Capture Options")
        self.layout = QVBoxLayout(self)
        self.setLayout(self.layout)

        self.form_layout = QFormLayout()
        self.form_layout.setSpacing(20)
        self.form_layout.setVerticalSpacing(20)
        self.layout.addItem(self.form_layout)

        self.location_layout = QHBoxLayout()
        self.location_label = QLabel("Where to save images/videos:", self)
        self.location_edit = QLineEdit(self)
        self.location_dialog_button = QPushButton("...", self)
        self.location_dialog_button.clicked.connect(self.open_file_dialog)
        self.location_layout.addWidget(self.location_edit)
        self.location_layout.addWidget(self.location_dialog_button)

        # maintain descriptions as own labels
        # pyqt seems to loose the descriptions somewhere
        # when simple strings are used or the qlabel do ot have self as owner
        self.form_layout.addRow(self.location_label,
                                self.location_layout)

        self.image_type_combobox = QComboBox(self)
        for key, value in encoder_dict.items():
            if value.encoder_type == Encoder.EncoderType.image:
                self.image_type_combobox.addItem(key)
        self.image_type_label = QLabel("Save images as:", self)
        self.form_layout.addRow(self.image_type_label,
                                self.image_type_combobox)
        if self.enabled_video:
            self.video_type_combobox = QComboBox(self)
            for key, value in encoder_dict.items():
                if value.encoder_type == Encoder.EncoderType.video:
                    self.video_type_combobox.addItem(key)
            self.video_type_label = QLabel("Save videos as:", self)
            self.form_layout.addRow(self.video_type_label,
                                    self.video_type_combobox)

        self.device_dialog_checkbox = QCheckBox(self)
        self.device_dialog_label = QLabel("Open device dialog on start:", self)
        self.form_layout.addRow(self.device_dialog_label,
                                self.device_dialog_checkbox)

        self.reopen_device_checkbox = QCheckBox(self)
        self.reopen_device_label = QLabel("Reopen device on start(ignores device dialog):", self)
        self.form_layout.addRow(self.reopen_device_label,
                                self.reopen_device_checkbox)


        # self.set_device_properties_checkbox = QCheckBox(self)
        # set_device_properties_label = QLabel("Set properties to cached values when known device is opened:", self)
        # self.form_layout.addRow(set_device_properties_label,
        #                         self.set_device_properties_checkbox)

        self.use_dutils_checkbox = QCheckBox(self)
        self.use_dutils_label = QLabel("Use tiscamera dutils, if present:", self)
        self.form_layout.addRow(self.use_dutils_label,
                                self.use_dutils_checkbox)

        if not self.enabled_dutils:
            self.use_dutils_label.setToolTip("Enabled when tiscamera-dutils are installed")
            self.use_dutils_label.setEnabled(False)
            self.use_dutils_checkbox.setToolTip("Enabled when tiscamera-dutils are installed")
            self.use_dutils_checkbox.setEnabled(False)

        # OK and Cancel buttons
        self.buttons = QDialogButtonBox(
            QDialogButtonBox.Reset | QDialogButtonBox.Ok | QDialogButtonBox.Cancel,
            Qt.Horizontal, self)
        self.layout.addWidget(self.buttons)

        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)
        self.buttons.clicked.connect(self.clicked)

    def set_settings(self, settings: Settings):
        self.location_edit.setText(settings.get_save_location())
        self.image_type_combobox.setCurrentText(settings.get_image_type())
        if self.enabled_video:
            self.video_type_combobox.setCurrentText(settings.get_video_type())
        self.device_dialog_checkbox.setChecked(settings.show_device_dialog_on_startup)
        self.reopen_device_checkbox.setChecked(settings.reopen_device_on_startup)
        # self.set_device_properties_checkbox.setChecked(settings.set_properties_on_reopen)
        self.use_dutils_checkbox.setChecked(settings.use_dutils)

    def save_settings(self):
        self.settings.save_location = self.location_edit.text()
        self.settings.image_type = self.image_type_combobox.currentText()
        if self.enabled_video:
            self.settings.video_type = self.video_type_combobox.currentText()
        self.settings.show_device_dialog_on_startup = self.device_dialog_checkbox.isChecked()
        self.settings.reopen_device_on_startup = self.reopen_device_checkbox.isChecked()
        # self.settings.set_properties_on_reopen = self.set_device_properties_checkbox.isChecked()
        self.settings.use_dutils = self.use_dutils_checkbox.isChecked()

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

    def clicked(self, button):

        if self.buttons.buttonRole(button) == QDialogButtonBox.ResetRole:
            self.reset()

    def reset(self):
        """"""
        log.info("reset called")
        self.settings.reset()
        self.set_settings(self.settings)

    @staticmethod
    def get_options(settings, parent=None):
        dialog = OptionsDialog(settings, parent)

        if settings is not None:
            dialog.set_settings(settings)
        result = dialog.exec_()

        if result == QDialog.Accepted:
            dialog.save_settings()
            settings.save()

        return result == QDialog.Accepted
