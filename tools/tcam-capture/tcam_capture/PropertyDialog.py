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
                             QFormLayout, QPushButton, QScrollArea)

from PyQt5.QtCore import QObject, pyqtSignal

import logging

log = logging.getLogger(__name__)


class PropertyTree(QWidget):
    """
    """

    request_visibility = pyqtSignal(str, bool)

    def __init__(self, data, parent=None):
        super(PropertyTree, self).__init__(parent)
        self.tcam = data.tcam
        self.data = data
        self.setup_ui()
        self.property_number = 0
        self.prop_dict = {}

    def setup_ui(self):
        self.formlayout = QFormLayout()
        self.formlayout.setSpacing(0)
        self.formlayout.setVerticalSpacing(0)

        self.layout = QVBoxLayout()
        self.layout.addLayout(self.formlayout)
        self.setLayout(self.layout)

    def finish_setup(self):
        # insert spacer only after all other elements have been added
        # self.layout.insertStretch(-1, 1)

        self.layout.addStretch()
        self.setLayout(self.layout)

    def add_property(self, prop: Prop):
        self.prop_dict[prop.name] = PropertyWidget(self.data, prop)
        # if not self.__add_to_roi_group_maybe(self.prop_dict[prop.name]):
        self.formlayout.addRow(prop.name, self.prop_dict[prop.name])

        self.property_number = self.property_number + 1

    def set_property(self, name, value):
        return self.tcam.set_tcam_property(name, value)

    def get_property_count(self):
        return self.property_number

    def update(self, prop):
        try:
            self.prop_dict[prop.name].update(prop)
        except KeyError:
            self.add_property(prop)

            if not self.isVisible():
                self.request_visibility.emit(prop.category, True)

    def reset(self):
        for key, item in self.prop_dict.items():
            item.reset()


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
        self.run = True

    def stop(self):
        self.run = False
        self.timer.stop()

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
                 category,
                 group) = self.tcam.get_tcam_property(name)
            except TypeError as e:
                # log.warning("get_tcam_property failed for '{}'".format(name))
                # log.("get_tcam_property failed for '{}'".format(name))
                continue

            prop = Prop(name, value, minval, maxval, defval, step, valuetype,
                        flags, category, group)

            prop_list.append(prop)
        return prop_list

    def update(self):
        if not self.run:
            self.timer.stop()
            return
        for p in self.get_prop_list():
            self.category_dict[p.category].update(p)


class PropertyDialog(QWidget):

    # allows the disabling/enabling of tabs
    # after initial setup
    toggle_tab_visibility = pyqtSignal(str, bool)

    def __init__(self, data, display_area, parent=None):
        super(PropertyDialog, self).__init__(parent)
        self.setWindowTitle("Tcam-Capture Device Properties")
        self.data = data
        self.display_area = display_area
        self.visible_tabs = []
        self.work_thread = QtCore.QThread()

        self.setup_ui()

        self.work_thread.start()

    def stop(self):
        self.worker.stop()
        self.work_thread.quit()
        if not self.work_thread.wait(5000):  # wait 5 seconds
            log.error("Property Update Thread did not stop in time")
            self.work_thread.terminate()

        self.work_thread.wait()

    def setup_ui(self):

        self.tabs = QTabWidget()

        self.tabs.setTabPosition(QTabWidget.West)
        self.reset_button = QPushButton("Reset")
        self.reset_button.clicked.connect(self.reset)
        self.layout = QVBoxLayout()
        self.layout.addWidget(self.tabs)
        self.layout.addWidget(self.reset_button)

        self.tab_dict = {
            "Exposure": PropertyTree(self.data, self),
            "Color": PropertyTree(self.data, self),
            "Lens": PropertyTree(self.data, self),
            "Special": PropertyTree(self.data, self),
            "Partial Scan": PropertyTree(self.data, self),
            "Image": PropertyTree(self.data, self),
            "WDR": PropertyTree(self.data, self),
            "Unknown": PropertyTree(self.data, self)
        }

        for val in self.tab_dict.values():

            val.request_visibility.connect(self.__toggle_tab)

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
                log.warning(e)
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
                    "Lens", "Special", "WDR", "Partial Scan", "Unknown"]

        for t in tab_list:
            self.__add_tab(t)

        self.worker = PropertyWorker(self.data.tcam,
                                     self.data.signals,
                                     self.tab_dict,
                                     None)  # parent of movable objects must not exists
        self.worker.moveToThread(self.work_thread)

    def __add_tab(self, name):
        """
        Add tab with name to PropertyDialog
        """
        try:
            tab = self.tab_dict[name]

            if tab.get_property_count() > 0:
                tab.setVisible(True)
            else:
                log.debug("Tab {} has no properties".format(name))
                tab.setVisible(False)
                return

            # wrap in scrollarea to ensure accessability
            area = QScrollArea()
            area.setWidgetResizable(True)
            area.setWidget(tab)
            tab.finish_setup()

            # the order of these entries is equivalent to the tab order
            # in the application
            tab_list = ["Exposure", "Image", "Color",
                        "Lens", "Special", "WDR", "Partial Scan", "Unknown"]

            # we want the tabs in a certain sequence
            # since we do not know which of these
            # already exist we have to manually count all
            # visible tabs before us to get the correct index
            our_index = tab_list.index(name)
            pos = 0
            for t in range(our_index):

                if tab_list[t] in self.visible_tabs:
                    pos += 1

            log.debug(pos)
            self.tabs.insertTab(pos, area, name)

            area.setVisible(True)

            log.info("Added tab {}".format(name))
            log.debug("tab index count is {}".format(self.tabs.count()))
            self.visible_tabs.append(name)

        except KeyError as err:
            log.error(err)

    def __toggle_tab(self, tab_name: str, make_visible: str):
        """
        Slot for adding tab to PropertyDialog
        """

        try:
            self.__add_tab(tab_name)

        except TypeError as err:
            log.error("Unable to find property tab. {}".format(err))

    def reset(self):
        for key, t in self.tab_dict.items():
            t.reset()
