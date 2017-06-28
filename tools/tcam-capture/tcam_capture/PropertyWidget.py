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

from PyQt5 import QtWidgets
from PyQt5.QtWidgets import (QHBoxLayout, QSlider, QPushButton,
                             QCheckBox, QComboBox, QWidget, QSpacerItem)
from PyQt5.QtCore import Qt, pyqtSignal
from . import TcamSignal, TcamCaptureData
import logging

log = logging.getLogger(__name__)


class Prop(object):
    """Property container"""
    def __init__(self, name, value, minval, maxval, defval, step, valuetype,
                 flags, category, group):
        self.name = name
        self.value = value
        self.minval = minval
        self.maxval = maxval
        self.defval = defval
        self.step = step
        self.valuetype = valuetype
        self.flags = flags
        self.category = category
        self.group = group


class PropertyWidget(QWidget):
    """display widget for tcam property"""
    def __init__(self, data: TcamCaptureData, prop: Prop):
        super().__init__()
        self.tcam = data.tcam
        self.signals = data.signals
        self.prop = prop
        self.setup_ui()

    def __repr__(self):
        return repr((self.prop.name, self.prop.valuetype,
                     self.prop.category, self.prop.group))

    def setup_ui(self):
        self.layout = QHBoxLayout()

        if self.prop.name != self.prop.group:
            self.spacer = QSpacerItem(30, 10)
            self.layout.addItem(self.spacer)

        self.setLayout(self.layout)
        if self.prop.valuetype != "boolean":
            self.name_label = QtWidgets.QLabel(self.prop.name)
            self.layout.addWidget(self.name_label)
        if self.prop.valuetype == "integer":
            self.value_label = QtWidgets.QLabel(str(self.prop.value))
            self.layout.addWidget(self.value_label)
            self.sld = QSlider(Qt.Horizontal, self)
            self.sld.setFocusPolicy(Qt.NoFocus)
            self.sld.setRange(self.prop.minval, self.prop.maxval)
            self.sld.setValue(self.prop.value)
            self.sld.setGeometry(30, 40, 100, 30)
            self.sld.valueChanged[int].connect(self.set_property)
            self.layout.addWidget(self.sld)
        elif self.prop.valuetype == "double":
            self.value_label = QtWidgets.QLabel(str(self.prop.value))
            self.layout.addWidget(self.value_label)
            self.sld = QSlider(Qt.Horizontal, self)
            self.sld.setFocusPolicy(Qt.NoFocus)
            self.sld.setRange(self.prop.minval * 1000, self.prop.maxval * 1000)
            self.sld.valueChanged[int].connect(self.set_property)
            self.sld.setGeometry(30, 40, 100, 30)
            self.layout.addWidget(self.sld)
        elif self.prop.valuetype == "button":
            self.checkbox = QPushButton(self)
            self.checkbox.clicked.connect(self.set_property)
            self.layout.addWidget(self.checkbox)
        elif self.prop.valuetype == "boolean":
            self.toggle = QPushButton(self.prop.name)
            self.toggle.setCheckable(True)
            if self.prop.value:
                self.toggle.toggle()
            self.toggle.toggled.connect(self.button_clicked)
            self.layout.addWidget(self.toggle)
        elif self.prop.valuetype == "string":
            pass
        elif self.prop.valuetype == "enum":
            self.combo = QComboBox(self)
            entry_list = self.tcam.get_tcam_menu_entries(self.prop.name)

            for e in entry_list:
                self.combo.addItem(e)
                self.combo.setCurrentText(self.prop.value)
                self.combo.currentIndexChanged['QString'].connect(self.set_property)
                self.layout.addWidget(self.combo)

    def button_clicked(self):
        log.debug("button clicked")
        self.signals.change_property.emit(self.tcam, self.prop.name,
                                          self.toggle.isChecked(), self.prop.valuetype)

    def set_property(self, value):
        if self.prop.valuetype == "integer":
            self.value_label.setText(str(value))
        if self.prop.valuetype == "double":
            self.value_label.setText(str(value / 1000))
            self.signals.change_property.emit(self.tcam, self.prop.name,
                                              float(value) / 1000, self.prop.valuetype)
            return

        self.signals.change_property.emit(self.tcam, self.prop.name,
                                          value, self.prop.valuetype)

    def update(self, prop: Prop):
        self.prop = prop
        if self.prop.valuetype == "integer":
            self.value_label.setText(str(self.prop.value))
            self.sld.blockSignals(True)
            self.sld.setValue(self.prop.value)
            self.sld.blockSignals(False)
        elif self.prop.valuetype == "double":
            self.sld.blockSignals(True)
            self.value_label.setText("{:.3f}".format(self.prop.value))
            self.sld.setValue((self.prop.value * 1000))
            self.sld.blockSignals(False)

        elif self.prop.valuetype == "button":
            pass

        elif self.prop.valuetype == "boolean":

            if self.prop.value and not self.toggle.isChecked():
                self.toggle.blockSignals(True)
                self.toggle.toggle()
                self.toggle.blockSignals(False)

        elif self.prop.valuetype == "string":
            pass
        elif self.prop.valuetype == "enum":
            self.combo.blockSignals(True)
            self.combo.setCurrentText(prop.value)
            self.combo.blockSignals(False)
