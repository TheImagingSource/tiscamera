# Copyright 2019 The Imaging Source Europe GmbH
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

from PyQt5.QtWidgets import QSpinBox, QDoubleSpinBox, QLineEdit

import logging

log = logging.getLogger(__name__)


class TcamSpinBox(QSpinBox):
    """
    This is a custom wrapper around QSpinBox.
    The reason for this wrapper is to allow us to
    listen to events.

    This enables us to prevent interuptions of
    value edits by the user due to updates.
    """

    def __init__(self,
                 parent=None):
        super(QSpinBox, self).__init__(parent)
        self.is_active = False

        # there is no way to access the internal lineedit
        # in the pyqt wrapper. that is why we set our own lineedit.
        # With this we can access the signals to block updates
        # while the user is editing.
        self.line = QLineEdit()
        self.line.textEdited.connect(self.__activate_block_str)
        self.setLineEdit(self.line)

        super().editingFinished.connect(self.__release_block)

    def __activate_block_str(self, i):
        """"""
        self.is_active = True

    def mousePressEvent(self, event):
        """

        """
        self.is_active = True
        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        """
        Reset mouse related properties
        """
        self.is_active = False
        super().mouseReleaseEvent(event)

    def __text_changed(self, text):
        """
        """
        self.is_active = True

    def __text_finished(self):
        """
        Callback for super().editingFinished()
        """
        self.is_active = False

    def __release_block(self):
        """
        Callback for super().editingFinished()
        """
        self.is_active = False

    def active(self):
        """"""
        return self.is_active

    def setValue(self, value):
        """
        Wrapper around QSpinBox.setValue

        Additionally checks for user activity to block updates.
        """
        if self.active():
            return

        super().setValue(value)


class TcamDoubleSpinBox(QDoubleSpinBox):
    """
    This is a custom wrapper around QSpinBox.
    The reason for this wrapper is to allow us to
    listen to events.

    This enables us to prevent interuptions of
    value edits by the user due to updates.
    """

    def __init__(self,
                 parent=None):
        super(QDoubleSpinBox, self).__init__(parent)
        self.is_active = False

        # there is no way to access the internal lineedit
        # in the pyqt wrapper. that is why we set our own lineedit.
        # With this we can access the signals to block updates
        # while the user is editing.
        self.line = QLineEdit()
        self.line.textEdited.connect(self.__activate_block_str)
        self.setLineEdit(self.line)

        super().editingFinished.connect(self.__release_block)
        self.valueChanged[int].connect(self.__activate_block)

    def __activate_block(self, i):
        """"""
        self.is_active = True

    def __activate_block_str(self, i):
        """"""
        self.is_active = True

    def mousePressEvent(self, event):
        """

        """
        self.is_active = True
        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        """
        Reset mouse related properties
        """
        self.is_active = False
        super().mouseReleaseEvent(event)

    def __text_changed(self, text):
        self.is_active = True

    def __text_finished(self):
        """
        Callback for super().editingFinished()
        """
        self.is_active = False

    def __release_block(self):
        """
        Callback for super().editingFinished()
        """
        self.is_active = False

    def active(self):
        """"""
        return self.is_active

    def setValue(self, value):
        """
        Wrapper around QSpinBox.setValue

        Additionally checks for user activity to block updates.
        """
        if self.active():
            return

        super().setValue(value)
