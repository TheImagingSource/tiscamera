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


from PyQt5.QtWidgets import (QHBoxLayout, QSlider, QPushButton,
                             QCheckBox, QComboBox, QWidget,
                             QSpinBox, QDoubleSpinBox, QLabel)
from PyQt5.QtGui import QIcon
from PyQt5.QtCore import Qt, pyqtSignal
from . import TcamCaptureData, TcamSlider, TcamSpinBox
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

    value_changed = pyqtSignal(object)

    """display widget for tcam property"""
    def __init__(self, data: TcamCaptureData, prop: Prop,
                 app_property: bool=False):
        super().__init__()
        self.app_property = app_property
        self.tcam = data.tcam
        self.signals = data.signals
        self.prop = prop
        self.is_log = False
        self.setup_ui()

    def __repr__(self):
        return repr((self.prop.name, self.prop.valuetype,
                     self.prop.category, self.prop.group))

    def __setup_ui_boolean(self):
        """
        Helper function that contains all setup code for bool UIs
        """
        self.toggle = QCheckBox(self)
        self.toggle.setCheckable(True)
        if self.prop.value:
            self.toggle.toggle()
        self.toggle.toggled.connect(self.button_clicked)
        self.layout.addWidget(self.toggle)

    def __setup_ui_integer(self):
        """

        """

        self.value_box = TcamSpinBox.TcamSpinBox(self)
        try:
            self.value_box.setRange(self.prop.minval, self.prop.maxval)
        except OverflowError:
            log.error("Property {} reported a range "
                      "that could not be handled".format(self.prop.name))

        self.value_box.setSingleStep(self.prop.step)
        self.value_box.blockSignals(True)
        self.value_box.setValue(self.prop.value)
        self.value_box.blockSignals(False)
        self.value_box.valueChanged[int].connect(self.set_property_box)
        self.value_box.setKeyboardTracking(False)

        if self.is_log:
            log.debug("Adding log slider for {}".format(self.prop.name))
            self.sld = TcamSlider.TcamLogSlider(self)
            self.sld.valueLogChanged.connect(self.set_property)
        else:
            self.sld = TcamSlider.TcamSlider(self)
            self.sld.valueChanged[int].connect(self.set_property)

        self.sld.setFocusPolicy(Qt.NoFocus)
        try:
            self.sld.blockSignals(True)
            self.sld.setRange(self.prop.minval, self.prop.maxval)
            self.sld.setValue(self.prop.value)
            self.sld.blockSignals(False)

        except OverflowError:
            log.error("Property {} reported a range "
                      "that could not be handled".format(self.prop.name))
        self.sld.setSingleStep(self.prop.step)
        self.sld.doubleClicked.connect(self.reset)

        self.layout.addWidget(self.sld)
        self.layout.addWidget(self.value_box)

    def __setup_ui_double(self):
        """
        Helper function that contains all setup code for doube UIs
        """
        self.value_box = TcamSpinBox.TcamDoubleSpinBox(self)
        try:
            self.value_box.setRange(self.prop.minval, self.prop.maxval)
        except OverflowError:
            log.error("Property {} reported a range "
                      "that could not be handled".format(self.prop.name))

        self.value_box.setSingleStep(self.prop.step)
        self.value_box.blockSignals(True)
        self.value_box.setValue(self.prop.value)
        self.value_box.blockSignals(False)
        self.value_box.valueChanged[float].connect(self.set_property_box)
        self.value_box.setKeyboardTracking(False)

        if self.is_log:
            self.sld = TcamSlider.TcamLogSlider(self)
            self.sld.valueLogChanged.connect(self.set_property)
        else:
            self.sld = TcamSlider.TcamSlider(self)
            self.sld.valueChanged[int].connect(self.set_property)

        self.sld.setFocusPolicy(Qt.NoFocus)
        try:
            self.sld.setRange(self.prop.minval * 100, self.prop.maxval * 100)
            self.sld.blockSignals(True)
            self.sld.setValue(self.prop.value * 100)
            self.sld.blockSignals(False)

        except OverflowError:
            log.error("Property {} reported a range "
                      "that could not be handled".format(self.prop.name))
        self.sld.setSingleStep(self.prop.step * 100)
        self.sld.doubleClicked.connect(self.reset)

        self.sld.setGeometry(30, 40, 100, 30)
        self.layout.addWidget(self.sld)
        self.layout.addWidget(self.value_box)

    def setup_ui(self):
        self.layout = QHBoxLayout()

        def should_be_logarithmic(prop):
            """
            Returns True when the range is > 5000
            """
            if prop.valuetype == "integer":
                if (prop.maxval - prop.minval) >= 5000:
                    return True

            elif prop.valuetype == "double":
                if (prop.maxval - prop.minval) >= 5000:
                    return True

            return False

        self.setLayout(self.layout)

        self.is_log = should_be_logarithmic(self.prop)

        if self.prop.valuetype == "integer":

            self.__setup_ui_integer()

        elif self.prop.valuetype == "double":

            self.__setup_ui_double()

        elif self.prop.valuetype == "button":
            self.checkbox = QPushButton(self)
            self.checkbox.clicked.connect(self.set_property)
            self.layout.addWidget(self.checkbox)

        elif self.prop.valuetype == "boolean":
            self.__setup_ui_boolean()

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
        self.value_changed.emit(self)

    def set_property(self, value, emit_value_changed=True):
        self.prop.value = value
        if self.prop.valuetype == "integer":
            self.update_box_value(self.value_box, value)

        if self.prop.valuetype == "double":
            self.update_box_value(self.value_box, value / 100)

            self.signals.change_property.emit(self.tcam, self.prop.name,
                                              float(value / 100), self.prop.valuetype)
            return

        self.signals.change_property.emit(self.tcam, self.prop.name,
                                          value, self.prop.valuetype)
        if emit_value_changed:
            self.value_changed.emit(self)

    def set_property_box(self, value):
        if self.prop.valuetype == "integer":
            self.update_slider_value(self.sld, value)

        if self.prop.valuetype == "double":
            self.update_slider_value(self.sld, value)

            self.signals.change_property.emit(self.tcam, self.prop.name,
                                              float(value), self.prop.valuetype)
            return

        self.signals.change_property.emit(self.tcam, self.prop.name,
                                          value, self.prop.valuetype)
        self.value_changed.emit(self)

    def update_box_value(self, box, value):
        box.blockSignals(True)
        box.setValue(value)
        box.blockSignals(False)

    def update_box_range(self, box, minval, maxval):
        """"""
        box.blockSignals(True)
        box.setRange(self.prop.minval,
                     self.prop.maxval)
        box.blockSignals(False)

    def update_slider_value(self, slider, value):
        slider.blockSignals(True)
        try:
            if self.prop.valuetype == "double":
                slider.setValue(value * 100)
            else:
                slider.setValue(value)

        except OverflowError:
            log.error("The slider for '{}' had a value outside of the integer "
                      "range. That should no happen.".format(self.prop.name))
        finally:
            slider.blockSignals(False)

    def update_slider_range(self, slider, minval, maxval):
        """"""
        self.sld.blockSignals(True)
        try:
            if self.prop.valuetype == "double":
                self.sld.setRange(self.prop.minval * 100,
                                  self.prop.maxval * 100)
            else:
                self.sld.setRange(self.prop.minval,
                                  self.prop.maxval)
        except OverflowError:
            log.error("The slider for '{}' had a value outside of the integer "
                      "range. That should no happen. "
                      "Min: '{}' Max: {}".format(self.prop.name,
                                                 self.prop.minval,
                                                 self.prop.maxval))
        finally:
            self.sld.blockSignals(False)

    def update(self, prop: Prop):

        emit_value_changed = False

        if self.prop.value != prop.value:
            emit_value_changed = True

        self.prop = prop
        if self.prop.valuetype == "integer":
            if type(self.value_box) is TcamSpinBox.TcamSpinBox:
                if self.value_box.active():
                    # box.setValue(value)
                    return
            self.update_slider_value(self.sld, self.prop.value)

            self.update_slider_range(self.sld,
                                     self.prop.minval,
                                     self.prop.maxval)
            self.update_box_range(self.value_box,
                                  self.prop.minval,
                                  self.prop.maxval)
            self.update_box_value(self.value_box, self.prop.value)
        elif self.prop.valuetype == "double":
            self.update_slider_range(self.sld,
                                     self.prop.minval,
                                     self.prop.maxval)
            self.update_slider_value(self.sld, self.prop.value)
            self.update_box_range(self.value_box,
                                  self.prop.minval,
                                  self.prop.maxval)
            self.update_box_value(self.value_box, self.prop.value)

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

        if emit_value_changed:
            self.value_changed.emit(self)

    def reset(self):
        if self.prop.valuetype == "integer":
            self.update_box_value(self.value_box, self.prop.defval)
            self.sld.setValue(self.prop.defval)

        elif self.prop.valuetype == "double":
            self.update_box_value(self.value_box, self.prop.defval)
            self.sld.setValue(self.prop.defval)

        elif self.prop.valuetype == "button":
            pass

        elif self.prop.valuetype == "boolean":
            if self.prop.defval and not self.toggle.isChecked():
                self.toggle.toggle()

        elif self.prop.valuetype == "string":
            pass
        elif self.prop.valuetype == "enum":
            self.combo.setCurrentText(self.prop.defval)
        self.value_changed.emit(self)
