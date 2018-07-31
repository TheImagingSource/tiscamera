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


from tcam_capture.PropertyWidget import PropertyWidget

from PyQt5.QtCore import QSizeF, QPoint


class ROIGroup(object):
    """"""
    def __init__(self):
        """"""
        self.name = ""
        # list of members
        self.member_names = []
        # list of matching PropertyWidgets
        self.properties = []
        # color the border should have
        self.border_color = "red"

    def is_complete(self):
        """
        Returns True if all members are available
        """
        if len(self.member_names) != len(self.properties):
            return False

        # Check if we really have all properties
        for name in self.member_names:
            if not any(name == prop.prop.name for prop in self.properties):
                return False

        return True

    def add_member_maybe(self, prop: PropertyWidget):
        """
        Check if property is an expected member and add it if so
        Returns True if property is a member
        """
        if prop.prop.name in self.member_names:
            self.properties.append(prop)
            return True
        return False

    def set_position(self, x: int, y: int):

        for prop in self.properties:
            if "Top" in prop.prop.name:
                prop.set_property(int(y), False)
            elif "Left" in prop.prop.name:
                prop.set_property(int(x), False)

    def set_size(self, width: int, height: int):

        for prop in self.properties:
            if "Width" in prop.prop.name:
                prop.set_property(int(width), False)
            elif "Height" in prop.prop.name:
                prop.set_property(int(height), False)

    def get_position(self):

        x = 0
        y = 0

        for prop in self.properties:

            if "Left" in prop.prop.name:
                x = prop.prop.value
            elif "Top" in prop.prop.name:
                y = prop.prop.value

        return QPoint(x, y)

    def get_size(self):

        width = 0
        height = 0

        for prop in self.properties:

            if "Width" in prop.prop.name:
                width = prop.prop.value
            elif "Height" in prop.prop.name:
                height = prop.prop.value

        return QSizeF(width, height)

    def get_min_size(self):
        """
        Return a QSizeF containing the minimum size this ROI must have
        """
        width = 0
        height = 0

        for prop in self.properties:

            if "Width" in prop.prop.name:
                width = prop.prop.minval
            elif "Height" in prop.prop.name:
                height = prop.prop.minval

        return QSizeF(width, height)

    def get_max_size(self):
        """
        Return the maximum possible size the ROI can have
        """
        width = 0
        height = 0

        for prop in self.properties:

            if "Width" in prop.prop.name:
                width = prop.prop.maxval
            elif "Height" in prop.prop.name:
                height = prop.prop.maxval

        return QSizeF(width, height)

    @staticmethod
    def get_all_groups():
        """
        Returns a list of all possible ROIGroups
        """
        roi_list = []

        exp1 = ROIGroup()
        exp1.name = "Exposure ROI"
        exp1.member_names = ["Exposure ROI Left", "Exposure ROI Top",
                             "Exposure ROI Width", "Exposure ROI Height"]
        roi_list.append(exp1)

        focus = ROIGroup()
        focus.name = "Focus ROI"
        focus.member_names = ["Focus ROI Left", "Focus ROI Top",
                              "Focus ROI Width", "Focus ROI Height"]
        focus.border_color = "blue"
        roi_list.append(focus)

        # devices like the DFK33 UX250 have this
        auto_roi = ROIGroup()
        auto_roi.name = "Auto Functions ROI"
        auto_roi.member_names = ["Auto Functions ROI Left", "Auto Functions ROI Top",
                                 "Auto Functions ROI Width", "Auto Functions ROI Height",
                                 "Auto Functions ROI Control", "Auto Functions ROI Preset"]
        auto_roi.border_color = "green"
        roi_list.append(auto_roi)

        return roi_list
