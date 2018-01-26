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

from tcam_capture.PropertyWidget import PropertyWidget, Prop
from PyQt5 import QtCore
from PyQt5.QtWidgets import (QWidget, QVBoxLayout, QTabWidget,
                             QFormLayout, QPushButton)

from PyQt5.QtCore import QObject

import logging

log = logging.getLogger(__name__)


class PropertyTree(QWidget):

    def __init__(self, data, parent=None):
        super(PropertyTree, self).__init__(parent)
        self.tcam = data.tcam
        self.data = data
        self.setup_ui()
        self.property_number = 0
        self.prop_dict = {}

    def setup_ui(self):
        self.layout = QFormLayout()
        self.layout.setSpacing(0)
        self.layout.setVerticalSpacing(0)
        self.setLayout(self.layout)

    def finish_setup(self):
        # insert spacer only after all other elements have been added
        # self.layout.insertStretch(-1, 1)
        pass

    def add_property(self, prop: Prop):
        self.prop_dict[prop.name] = PropertyWidget(self.data, prop)
        self.layout.addRow(prop.name, self.prop_dict[prop.name])
        self.property_number = self.property_number + 1

    def set_property(self, name, value):
        return self.tcam.set_tcam_property(name, value)

    def get_property_count(self):
        return self.property_number

    def update(self, prop):
        try:
            self.prop_dict[prop.name].update(prop)
        except KeyError as e:
            self.add_property(prop)



class PropertyWorker(QObject):
    def __init__(self, tcam, signals, category_dict, parent=None):
        super(PropertyWorker, self).__init__(parent)
        self.signals = signals
        self.signals.change_property.connect(self.set_property)

        self.tcam = tcam
        self.category_dict = category_dict
        self.sleep_period_seconds = 2
        self.timer = QtCore.QTimer()
        self.timer.timeout.connect(self.update)
        self.timer.start(self.sleep_period_seconds * 1000)

    def set_property(self, tcam, name, value, valuetype):
        if (valuetype == "integer" or
            valuetype == "double" or
            valuetype == "boolean"):
            tcam.set_tcam_property(name, value)
        elif valuetype == "button":
            tcam.set_tcam_property(name, True)
        elif valuetype == "string":
            pass
        elif valuetype == "enum":
            tcam.set_tcam_property(name, value)

    def get_prop_list(self):
        prop_names = self.tcam.get_tcam_property_names()
        prop_list = []
        for name in prop_names:
            try:
                (result, value,
                 minval, maxval,
                 defval, step,
                 valuetype,
                 flags,
                 category, group) = self.tcam.get_tcam_property(name)
            except TypeError as e:
                # log.warning("get_tcam_property failed for '{}'".format(name))
                # log.("get_tcam_property failed for '{}'".format(name))
                continue

            prop = Prop(name, value, minval, maxval, defval, step, valuetype,
                        flags, category, group)

            prop_list.append(prop)
        return prop_list

    def update(self):
        for p in self.get_prop_list():
            self.category_dict[p.category].update(p)


class PropertyDialog(QWidget):

    def __init__(self, data, parent=None):
        super(PropertyDialog, self).__init__(parent)
        self.setWindowTitle("Tcam-Capture Device Properties")
        self.data = data
        self.work_thread = QtCore.QThread()

        self.setup_ui()

        self.work_thread.start()

    def __del__(self):
        self.work_thread.quit()
        self.work_thread.wait()

    def setup_ui(self):

        self.tabs = QTabWidget()

        self.tabs.setTabPosition(QTabWidget.West)
        self.reset_button = QPushButton("Reset")
        self.reset_button.clicked.connect(self.reset)
        self.layout = QVBoxLayout()
        self.layout.addWidget(self.tabs)

        self.tab_dict = {
            "Exposure": PropertyTree(self.data, self),
            "Color": PropertyTree(self.data, self),
            "Lens": PropertyTree(self.data, self),
            "Special": PropertyTree(self.data, self),
            "Partial Scan": PropertyTree(self.data, self),
            "Image": PropertyTree(self.data, self),
            "Unknown": PropertyTree(self.data, self)
        }
        prop_names = self.data.tcam.get_tcam_property_names()

        for name in prop_names:
            try:
                (result, value,
                 minval, maxval,
                 defval, step,
                 valuetype,
                 flags,
                 category, group) = self.data.tcam.get_tcam_property(name)
            except TypeError as e:
                log.warning("get_tcam_property failed for '{}'".format(name))
                continue

            prop = Prop(name, value, minval, maxval, defval, step, valuetype,
                        flags, category, group)

            try:
                self.tab_dict[category].add_property(prop)
            except KeyError as e:
                self.tab_dict["Unknown"].add_property(prop)

        self.setLayout(self.layout)

        # the order of these entries is equivalent to the tab order
        # in the application
        tab_list = ["Exposure", "Image", "Color",
                    "Lens", "Special", "Partial Scan", "Unknown"]

        for t in tab_list:
            tab = self.tab_dict[t]
            if tab.get_property_count() > 0:
                self.tabs.addTab(tab, t)
                tab.finish_setup()
            else:
                tab.setVisible(False)

        self.worker = PropertyWorker(self.data.tcam,
                                     self.data.signals,
                                     self.tab_dict,
                                     None)  # parent of movable objects must not exists
        self.worker.moveToThread(self.work_thread)
