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
from .Settings import Settings, FileNameSettings

import logging

from PyQt5.QtWidgets import (QDialog, QVBoxLayout,
                             QLabel, QLineEdit, QComboBox,
                             QDialogButtonBox, QFileDialog,
                             QPushButton, QHBoxLayout, QCheckBox,
                             QFormLayout, QTabWidget, QWidget,
                             QGroupBox, QSpinBox, QKeySequenceEdit)
from PyQt5.QtCore import Qt
from PyQt5.QtGui import QKeySequence

log = logging.getLogger(__file__)


class OptionsDialog(QDialog):
    def __init__(self, setting: Settings, have_dutils, parent=None):
        super(OptionsDialog, self).__init__(parent)

        self.settings = setting
        self.enabled_video = True  # temporary toggle to disable video features as they do not exist
        self.enabled_logging = True
        self.enabled_keybindings = True
        self.enabled_dutils = have_dutils
        self.setWindowTitle("Tcam-Capture Options")
        self.layout = QVBoxLayout(self)
        self.setLayout(self.layout)

        self.tabs = QTabWidget()

        self.general_widget = QWidget()
        self.keybindings_widget = QWidget()
        self.logging_widget = QWidget()
        self.saving_widget = QWidget()

        self._setup_general_ui()
        self.tabs.addTab(self.general_widget, "General")

        if self.enabled_keybindings:
            self._setup_keybindings_ui()
            self.tabs.addTab(self.keybindings_widget, "Keybindings")
        self._setup_saving_ui()
        self.tabs.addTab(self.saving_widget, "Image/Video")

        self.layout.addWidget(self.tabs)
        # OK and Cancel buttons
        self.buttons = QDialogButtonBox(
            QDialogButtonBox.Reset | QDialogButtonBox.Ok | QDialogButtonBox.Cancel,
            Qt.Horizontal, self)
        self.layout.addWidget(self.buttons)

        self.buttons.accepted.connect(self.accept)
        self.buttons.rejected.connect(self.reject)
        self.buttons.clicked.connect(self.clicked)

    def _setup_general_ui(self):
        """
        Create everything related to the general tab
        """

        layout = QFormLayout()
        layout.setSpacing(20)
        layout.setVerticalSpacing(20)

        self.device_dialog_checkbox = QCheckBox(self)
        device_dialog_label = QLabel("Open device dialog on start:")
        layout.addRow(device_dialog_label,
                      self.device_dialog_checkbox)

        self.reopen_device_checkbox = QCheckBox(self)
        reopen_device_label = QLabel("Reopen device on start(ignores device dialog):", self)
        layout.addRow(reopen_device_label,
                      self.reopen_device_checkbox)

        self.use_dutils_checkbox = QCheckBox(self)
        self.use_dutils_label = QLabel("Use tiscamera dutils, if present:", self)
        layout.addRow(self.use_dutils_label,
                      self.use_dutils_checkbox)

        if not self.enabled_dutils:
            self.use_dutils_label.setToolTip("Enabled when tiscamera-dutils are installed")
            self.use_dutils_label.setEnabled(False)
            self.use_dutils_checkbox.setToolTip("Enabled when tiscamera-dutils are installed")
            self.use_dutils_checkbox.setEnabled(False)

        self.general_widget.setLayout(layout)

    def _setup_saving_ui(self):
        """
        Create everything related to the image/video saving tab
        """
        encoder_dict = Encoder.get_encoder_dict()
        form_layout = QFormLayout()

        layout = QVBoxLayout()
        layout.addLayout(form_layout)

        location_layout = QHBoxLayout()
        location_label = QLabel("Where to save images/videos:", self)
        self.location_edit = QLineEdit(self)
        location_dialog_button = QPushButton("...", self)
        location_dialog_button.clicked.connect(self.open_file_dialog)
        location_layout.addWidget(self.location_edit)
        location_layout.addWidget(location_dialog_button)

        # maintain descriptions as own labels
        # pyqt seems to loose the descriptions somewhere
        # when simple strings are used or the qlabel does not have self as owner
        form_layout.addRow(location_label,
                           location_layout)

        self.image_type_combobox = QComboBox(self)
        for key, value in encoder_dict.items():
            if value.encoder_type == Encoder.MediaType.image:
                self.image_type_combobox.addItem(key)
        image_type_label = QLabel("Save images as:")
        self.image_type_combobox.currentIndexChanged['QString'].connect(self.image_name_suffix_changed)

        form_layout.addRow(image_type_label,
                           self.image_type_combobox)
        if self.enabled_video:
            self.video_type_combobox = QComboBox(self)
            for key, value in encoder_dict.items():
                if value.encoder_type == Encoder.MediaType.video:
                    self.video_type_combobox.addItem(key)
            self.video_type_combobox.currentIndexChanged['QString'].connect(self.video_name_suffix_changed)

            video_type_label = QLabel("Save videos as:", self)
            form_layout.addRow(video_type_label,
                               self.video_type_combobox)

        image_name_groupbox = QGroupBox("Image File Names")
        groupbox_layout = QFormLayout()
        image_name_groupbox.setLayout(groupbox_layout)

        self.image_name_preview = QLabel("<USER-PREFIX>-<SERIAL>-<FORMAT>-<TIMESTAMP>-<COUNTER>.png")
        self.image_name_preview_description = QLabel("Images will be named like:")
        groupbox_layout.addRow(self.image_name_preview_description,
                               self.image_name_preview)

        self.image_name_prefix = QLineEdit()
        self.image_name_prefix.textChanged.connect(self.image_name_prefix_changed)
        self.image_name_prefix.setMaxLength(100)

        self.image_name_prefix_description = QLabel("User Prefix:", self)
        groupbox_layout.addRow(self.image_name_prefix_description,
                               self.image_name_prefix)

        self.image_name_serial = QCheckBox(self)
        self.image_name_serial.toggled.connect(self.image_name_properties_toggled)
        self.image_name_serial_description = QLabel("Include Serial:")
        groupbox_layout.addRow(self.image_name_serial_description,
                               self.image_name_serial)

        self.image_name_format = QCheckBox(self)
        self.image_name_format.toggled.connect(self.image_name_properties_toggled)

        self.image_name_format_description = QLabel("Include Format:")
        groupbox_layout.addRow(self.image_name_format_description,
                               self.image_name_format)

        self.image_name_counter = QCheckBox(self)
        self.image_name_counter.toggled.connect(self.image_name_properties_toggled)
        self.image_name_counter_description = QLabel("Include Counter:")
        groupbox_layout.addRow(self.image_name_counter_description,
                               self.image_name_counter)

        self.image_name_counter_box = QSpinBox(self)
        self.image_name_counter_box.setRange(1, 10)
        self.image_name_counter_box.valueChanged.connect(self.image_name_counter_changed)
        self.image_name_counter_box_description = QLabel("Counter Size:")
        groupbox_layout.addRow(self.image_name_counter_box_description,
                               self.image_name_counter_box)

        self.image_name_counter.toggled.connect(self.toggle_image_counter_box_availability)
        self.image_name_counter.toggled.connect(self.image_name_properties_toggled)

        self.image_name_timestamp = QCheckBox(self)
        self.image_name_timestamp.toggled.connect(self.image_name_properties_toggled)
        self.image_name_timestamp_description = QLabel("Include Timestamp:")
        groupbox_layout.addRow(self.image_name_timestamp_description,
                               self.image_name_timestamp)

        layout.addWidget(image_name_groupbox)

        video_groupbox = QGroupBox("Video File Names")

        video_layout = QFormLayout()
        video_groupbox.setLayout(video_layout)

        self.video_name_preview = QLabel("<USER-PREFIX>-<SERIAL>-<FORMAT>-<TIMESTAMP>-<COUNTER>.png")
        self.video_name_preview_description = QLabel("Videos will be named like:")
        video_layout.addRow(self.video_name_preview_description,
                            self.video_name_preview)

        self.video_name_prefix = QLineEdit()
        self.video_name_prefix.textChanged.connect(self.video_name_prefix_changed)
        self.video_name_prefix.setMaxLength(100)

        self.video_name_prefix_description = QLabel("User Prefix:", self)
        video_layout.addRow(self.video_name_prefix_description,
                            self.video_name_prefix)

        self.video_name_serial = QCheckBox(self)
        self.video_name_serial.toggled.connect(self.video_name_properties_toggled)
        self.video_name_serial_description = QLabel("Include Serial:")
        video_layout.addRow(self.video_name_serial_description,
                            self.video_name_serial)

        self.video_name_format = QCheckBox(self)
        self.video_name_format.toggled.connect(self.video_name_properties_toggled)

        self.video_name_format_description = QLabel("Include Format:")
        video_layout.addRow(self.video_name_format_description,
                            self.video_name_format)

        self.video_name_counter = QCheckBox(self)
        self.video_name_counter.toggled.connect(self.video_name_properties_toggled)
        self.video_name_counter_description = QLabel("Include Counter:")
        video_layout.addRow(self.video_name_counter_description,
                            self.video_name_counter)

        self.video_name_counter_box = QSpinBox(self)
        self.video_name_counter_box.setRange(1, 10)
        self.video_name_counter_box.valueChanged.connect(self.video_name_counter_changed)
        self.video_name_counter_box_description = QLabel("Counter Size:")
        video_layout.addRow(self.video_name_counter_box_description,
                            self.video_name_counter_box)

        self.video_name_counter.toggled.connect(self.toggle_video_counter_box_availability)
        self.video_name_counter.toggled.connect(self.video_name_properties_toggled)

        self.video_name_timestamp = QCheckBox(self)
        self.video_name_timestamp.toggled.connect(self.video_name_properties_toggled)
        self.video_name_timestamp_description = QLabel("Include Timestamp:")
        video_layout.addRow(self.video_name_timestamp_description,
                            self.video_name_timestamp)

        layout.addWidget(video_groupbox)

        self.saving_widget.setLayout(layout)

    def image_name_prefix_changed(self, name: str):
        """"""

        self.settings.image_name.user_prefix = self.image_name_prefix.text()
        self.update_image_name_preview()

    def image_name_suffix_changed(self, suffix: str):
        """"""

        self.update_image_name_preview()

    def image_name_counter_changed(self, name: str):
        """"""
        self.settings.image_name.counter_size = self.image_name_counter_box.value()
        self.update_image_name_preview()

    def image_name_properties_toggled(self):
        """"""

        self.settings.image_name.include_timestamp = self.image_name_timestamp.isChecked()
        self.settings.image_name.include_counter = self.image_name_counter.isChecked()
        self.settings.image_name.include_format = self.image_name_format.isChecked()
        self.settings.image_name.include_serial = self.image_name_serial.isChecked()

        self.update_image_name_preview()

    def update_image_name_preview(self):

        preview_string = ""

        if self.settings.image_name.user_prefix != "":

            max_prefix_length = 15
            prefix = (self.settings.image_name.user_prefix[:max_prefix_length] + '..') if len(self.settings.image_name.user_prefix) > max_prefix_length else self.settings.image_name.user_prefix

            preview_string += prefix

        if self.settings.image_name.include_serial:
            if preview_string != "":
                preview_string += "-"
            preview_string += "00001234"

        if self.settings.image_name.include_format:
            if preview_string != "":
                preview_string += "-"
            preview_string += "gbrg_1920x1080_15_1"

        if self.settings.image_name.include_timestamp:
            if preview_string != "":
                preview_string += "-"
            preview_string += "19701230T125503"

        if self.settings.image_name.include_counter:
            if preview_string != "":
                preview_string += "-"
            preview_string += '{message:0>{fill}}'.format(message=1,
                                                          fill=self.settings.image_name.counter_size)

        if preview_string == "":
            preview_string = "image"

        preview_string += "." + self.image_type_combobox.currentText()

        self.image_name_preview.setText(preview_string)


    def video_name_prefix_changed(self, name: str):
        """"""

        self.settings.video_name.user_prefix = self.video_name_prefix.text()
        self.update_video_name_preview()

    def video_name_suffix_changed(self, suffix: str):
        """"""

        self.update_video_name_preview()

    def video_name_counter_changed(self, name: str):
        """"""
        self.settings.video_name.counter_size = self.video_name_counter_box.value()
        self.update_video_name_preview()

    def video_name_properties_toggled(self):
        """"""

        self.settings.video_name.include_timestamp = self.video_name_timestamp.isChecked()
        self.settings.video_name.include_counter = self.video_name_counter.isChecked()
        self.settings.video_name.include_format = self.video_name_format.isChecked()
        self.settings.video_name.include_serial = self.video_name_serial.isChecked()

        self.update_video_name_preview()

    def update_video_name_preview(self):

        preview_string = ""

        if self.settings.video_name.user_prefix != "":

            # This is a convenience change to the displayed string.
            # We only display an amount of max_prefix_length
            # chars to save screen space
            max_prefix_length = 15
            prefix = (self.settings.video_name.user_prefix[:max_prefix_length] + '..') if len(self.settings.video_name.user_prefix) > max_prefix_length else self.settings.video_name.user_prefix

            preview_string += prefix

        if self.settings.video_name.include_serial:
            if preview_string != "":
                preview_string += "-"
            preview_string += "00001234"

        if self.settings.video_name.include_format:
            if preview_string != "":
                preview_string += "-"
            preview_string += "gbrg_1920x1080_15_1"

        if self.settings.video_name.include_timestamp:
            if preview_string != "":
                preview_string += "-"
            preview_string += "19701230T125503"

        if self.settings.video_name.include_counter:
            if preview_string != "":
                preview_string += "-"
            preview_string += '{message:0>{fill}}'.format(message=1,
                                                          fill=self.settings.video_name.counter_size)

        if preview_string == "":
            preview_string = "video"

        preview_string += "." + self.video_type_combobox.currentText()

        self.video_name_preview.setText(preview_string)

    def toggle_image_counter_box_availability(self):
        """"""
        if self.image_name_counter.isChecked():
            self.image_name_counter_box.setEnabled(True)
        else:
            self.image_name_counter_box.setEnabled(False)

    def toggle_video_counter_box_availability(self):
        """"""
        if self.video_name_counter.isChecked():
            self.video_name_counter_box.setEnabled(True)
        else:
            self.video_name_counter_box.setEnabled(False)

    def _setup_keybindings_ui(self):
        """
        Create everything related to the keybindings tab
        """

        layout = QFormLayout()
        self.keybinding_fullscreen_label = QLabel("Toggle Fullscreen:")
        self.keybinding_fullscreen = QKeySequenceEdit()
        layout.addRow(self.keybinding_fullscreen_label,
                      self.keybinding_fullscreen)

        self.keybinding_save_image_label = QLabel("Save image:")
        self.keybinding_save_image = QKeySequenceEdit(QKeySequence(self.settings.keybinding_save_image))
        layout.addRow(self.keybinding_save_image_label,
                      self.keybinding_save_image)

        self.keybinding_trigger_image_label = QLabel("Trigger images via softwaretrigger:")
        self.keybinding_trigger_image = QKeySequenceEdit(QKeySequence(self.settings.keybinding_trigger_image))
        layout.addRow(self.keybinding_trigger_image_label,
                      self.keybinding_trigger_image)

        self.keybinding_open_dialog_label = QLabel("Open device dialog:")
        self.keybinding_open_dialog = QKeySequenceEdit(QKeySequence(self.settings.keybinding_open_dialog))
        layout.addRow(self.keybinding_open_dialog_label,
                      self.keybinding_open_dialog)

        self.keybindings_widget.setLayout(layout)

    def set_settings(self, settings: Settings):
        self.location_edit.setText(settings.get_save_location())
        self.image_type_combobox.setCurrentText(settings.get_image_type())
        if self.enabled_video:
            self.video_type_combobox.setCurrentText(settings.get_video_type())
        self.device_dialog_checkbox.setChecked(settings.show_device_dialog_on_startup)
        self.reopen_device_checkbox.setChecked(settings.reopen_device_on_startup)
        self.use_dutils_checkbox.setChecked(settings.use_dutils)

        #
        # keybindings
        #
        if self.enabled_keybindings:
            self.keybinding_fullscreen.setKeySequence(QKeySequence(self.settings.keybinding_fullscreen))
            self.keybinding_save_image.setKeySequence(QKeySequence(self.settings.keybinding_save_image))
            self.keybinding_trigger_image.setKeySequence(QKeySequence(self.settings.keybinding_trigger_image))
            self.keybinding_open_dialog.setKeySequence(QKeySequence(self.settings.keybinding_open_dialog))

        #
        # image saving
        #
        if settings.image_name.include_timestamp:
            self.image_name_timestamp.blockSignals(True)
            self.image_name_timestamp.toggle()
            self.image_name_timestamp.blockSignals(False)
        if settings.image_name.include_counter:
            self.image_name_counter.blockSignals(True)
            self.image_name_counter.toggle()
            self.image_name_counter.blockSignals(False)

        self.image_name_counter_box.blockSignals(True)
        self.image_name_counter_box.setValue(settings.image_name.counter_size)
        self.image_name_counter_box.blockSignals(False)
        self.toggle_image_counter_box_availability()

        if settings.image_name.include_format:
            self.image_name_format.blockSignals(True)
            self.image_name_format.toggle()
            self.image_name_format.blockSignals(False)
        if settings.image_name.include_serial:
            self.image_name_serial.blockSignals(True)
            self.image_name_serial.toggle()
            self.image_name_serial.blockSignals(False)
        self.image_name_prefix.blockSignals(True)
        self.image_name_prefix.setText(settings.image_name.user_prefix)
        self.image_name_prefix.blockSignals(False)

        self.update_image_name_preview()

        #
        # video saving
        #
        if settings.video_name.include_timestamp:
            self.video_name_timestamp.blockSignals(True)
            self.video_name_timestamp.toggle()
            self.video_name_timestamp.blockSignals(False)
        if settings.video_name.include_counter:
            self.video_name_counter.blockSignals(True)
            self.video_name_counter.toggle()
            self.video_name_counter.blockSignals(False)

        self.video_name_counter_box.blockSignals(True)
        self.video_name_counter_box.setValue(settings.video_name.counter_size)
        self.video_name_counter_box.blockSignals(False)
        self.toggle_video_counter_box_availability()

        if settings.video_name.include_format:
            self.video_name_format.blockSignals(True)
            self.video_name_format.toggle()
            self.video_name_format.blockSignals(False)
        if settings.video_name.include_serial:
            self.video_name_serial.blockSignals(True)
            self.video_name_serial.toggle()
            self.video_name_serial.blockSignals(False)
        self.video_name_prefix.blockSignals(True)
        self.video_name_prefix.setText(settings.video_name.user_prefix)
        self.video_name_prefix.blockSignals(False)

        self.update_video_name_preview()

    def save_settings(self):
        self.settings.save_location = self.location_edit.text()
        self.settings.image_type = self.image_type_combobox.currentText()
        if self.enabled_video:
            self.settings.video_type = self.video_type_combobox.currentText()
        self.settings.show_device_dialog_on_startup = self.device_dialog_checkbox.isChecked()
        self.settings.reopen_device_on_startup = self.reopen_device_checkbox.isChecked()
        self.settings.use_dutils = self.use_dutils_checkbox.isChecked()

        #
        # keybindings
        #
        if self.enabled_keybindings:
            self.settings.keybinding_fullscreen = self.keybinding_fullscreen.keySequence().toString()
            self.settings.keybinding_save_image = self.keybinding_save_image.keySequence().toString()
            self.settings.keybinding_trigger_image = self.keybinding_trigger_image.keySequence().toString()
            self.settings.keybinding_open_dialog = self.keybinding_open_dialog.keySequence().toString()

        #
        # image saving
        #
        self.settings.image_name.include_timestamp = self.image_name_timestamp.isChecked()
        self.settings.image_name.include_counter = self.image_name_counter.isChecked()
        if self.image_name_counter.isChecked():
            self.settings.image_name.counter_size = self.image_name_counter_box.value()

        self.settings.image_name.include_format = self.image_name_format.isChecked()
        self.settings.image_name.include_serial = self.image_name_serial.isChecked()
        self.settings.image_name.user_prefix = self.image_name_prefix.text()

        #
        # video saving
        #
        self.settings.video_name.include_timestamp = self.video_name_timestamp.isChecked()
        self.settings.video_name.include_counter = self.video_name_counter.isChecked()
        if self.video_name_counter.isChecked():
            self.settings.video_name.counter_size = self.video_name_counter_box.value()

        self.settings.video_name.include_format = self.video_name_format.isChecked()
        self.settings.video_name.include_serial = self.video_name_serial.isChecked()
        self.settings.video_name.user_prefix = self.video_name_prefix.text()

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
