# Copyright 2018 The Imaging Source Europe GmbH
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
from tcam_capture.ROIGroup import ROIGroup
from tcam_capture.ROIRectItem import ROIRectItem
from tcam_capture.ResizeableRectItem import ResizeableRectItem, ResizeableRectItemSettings

from PyQt5.QtWidgets import (QWidget, QVBoxLayout,
                             QGroupBox, QHBoxLayout, QFormLayout,
                             QPushButton, QCheckBox,
                             QLabel, QGraphicsItem, QSizePolicy)
from PyQt5.QtGui import QColor
from PyQt5.QtCore import pyqtSignal, QRect, QRectF

import logging

log = logging.getLogger(__name__)


class ROICollection(QWidget):
    """
    This widget contains the groubox that is responsible
    for ROI property display and ROI visualization and selection
    It creates the ROIRectItem for visualization
    Setting of the PropertyWidgets is done here.
    """

    selection_finished = pyqtSignal(QRect)

    def __init__(self, group: ROIGroup, parent=None):
        super(ROICollection, self).__init__(parent)
        self.group = group
        self.name = self.group.name

        log.info("Created Collection with name: {}".format(self.name))

        self.__setup_ui()
        self.display_area = None
        self.rubberband = None
        self.selection_active = False
        # bool telling us if the ROI rect was displayed before making a selection
        self.rect_was_displayed = False

        self.selection_finished.connect(self.apply_selection)

    def __setup_ui(self):

        self.layout = QVBoxLayout()
        self.groupbox = QGroupBox(self.name)

        self.setSizePolicy(QSizePolicy.Expanding,
                           QSizePolicy.Minimum)

        self.layout.addWidget(self.groupbox)

        self.__populate_groupbox()
        self.setLayout(self.layout)

    def __populate_groupbox(self):
        """

        """
        layout = QVBoxLayout()
        self.groupbox.setLayout(layout)

        first_line = QHBoxLayout()

        self.visibility_label = QLabel("Show ROI:")
        first_line.addWidget(self.visibility_label)
        self.visibility_checkbox = QCheckBox(self)
        self.visibility_checkbox.setCheckable(True)
        self.visibility_checkbox.toggled.connect(self.__checkbox_cb)
        first_line.addWidget(self.visibility_checkbox)
        self.select_button = QPushButton("+", self)
        self.select_button.setToolTip("Select ROI with the mouse")
        self.select_button.clicked.connect(self.activate_selection)
        first_line.addWidget(self.select_button)

        layout.addLayout(first_line)

        form_layout = QFormLayout()
        layout.addLayout(form_layout)

        for prop in self.group.properties:
            form_layout.addRow(prop.prop.name, prop)

    def __checkbox_cb(self):
        """
        SLOT for visibility_checkbox
        """
        self.toggle_visibility(self.visibility_checkbox.isChecked())

    def __add_roi_rect(self):
        """
        Creates a ROI Widgets and adds it to the display area
        """

        settings = ResizeableRectItemSettings(50,
                                              QColor(self.group.border_color),
                                              self.group.get_min_size(),
                                              self.group.get_max_size())

        rect = QRectF(self.group.get_position(),
                      self.group.get_size())

        self.rubberband = ROIRectItem(rect, settings, self.group)
        self.rubberband.setFlag(QGraphicsItem.ItemIsMovable)
        self.rubberband.position = self.group.get_position()
        self.rubberband.size = self.group.get_size()

        for prop in self.group.properties:
            prop.value_changed.connect(self.__update_rubberband_values)

        self.display_area.add_roi(self.rubberband)

    def __remove_rubberband(self):

        if not self.rubberband:
            return

        self.display_area.remove_roi(self.rubberband)
        self.rubberband = None

        for prop in self.group.properties:
            prop.value_changed.disconnect(self.__update_rubberband_values)

    def toggle_visibility(self,
                          be_visible: bool):
        """
        be_visible: bool saying if ROI should be visible as an overlay
        """

        if be_visible:

            if self.rubberband:
                self.__remove_rubberband()
            self.__add_roi_rect()
        else:
            self.__remove_rubberband()

    def activate_selection(self):
        """

        """
        if self.selection_active:
            return

        if self.rubberband:
            self.rect_was_displayed = True
            self.__remove_rubberband()

        self.display_area.start_roi_capture(self.selection_finished)
        self.selection_active = True

    def apply_selection(self, rect: QRect):
        self.selection_active = False

        self.group.set_position(rect.x(), rect.y())
        self.group.set_size(rect.width(), rect.height())

        if self.rect_was_displayed and self.visibility_checkbox.isChecked():
            self.__add_roi_rect()
        self.rect_was_displayed = False

    def __update_rubberband_values(self, prop: PropertyWidget):
        """
        SLOT for value_changed signal from the PropertyWidgets
        """

        if not self.rubberband:
            return

        if self.rubberband.mouse_pressed:
            return

        if ("Top" in prop.prop.name or
                "Left" in prop.prop.name):

            self.rubberband.position = self.group.get_position()
            if "Left" in prop.prop.name:

                self.rubberband.position.x = prop.prop.value

            elif "Top" in prop.prop.name:

                self.rubberband.position.y = prop.prop.value

            # log.info("scenePos{}".format(self.rubberband.scenePos()))
            # log.info("pos {}".format(self.group.get_position()))
            self.rubberband.update_pos()

        elif ("Width" in prop.prop.name or
                "Height" in prop.prop.name):

            if "Width" in prop.prop.name:
                if int(self.rubberband.size.width()) == prop.prop.value:
                    return

            if "Height" in prop.prop.name:
                if int(self.rubberband.size.height()) == prop.prop.value:
                    return

            self.rubberband.size = self.group.get_size()
            # log.info("size {}".format(self.group.get_size()))
            self.rubberband.update_rect()

        self.display_area.update()
