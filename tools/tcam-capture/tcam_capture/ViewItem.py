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

from PyQt5.QtWidgets import (QGraphicsPixmapItem)

from PyQt5.QtGui import QColor


class ViewItem(QGraphicsPixmapItem):
    """
    Derived class enables mouse tracking for color under mouse retrieval
    """

    def __init__(self, parent=None):
        super(ViewItem, self).__init__(parent)
        self.setAcceptHoverEvents(True)
        self.mouse_over = False  # flag if mouse is over our widget
        self.mouse_position_x = -1
        self.mouse_position_y = -1

    def hoverEnterEvent(self, event):
        self.mouse_over = True

    def hoverLeaveEvent(self, event):
        self.mouse_over = False

    def hoverMoveEvent(self, event):
        mouse_position = event.pos()

        self.mouse_position_x = mouse_position.x()
        self.mouse_position_y = mouse_position.y()
        super().hoverMoveEvent(event)

    def get_resolution(self):
        return self.pixmap().size()

    def get_mouse_color(self):
        if self.mouse_over:
            if(self.mouse_position_x <= self.pixmap().width() and
               self.mouse_position_y <= self.pixmap().height()):
                return QColor(self.pixmap().toImage().pixel(self.mouse_position_x,
                                                            self.mouse_position_y))
            else:
                self.mouse_position_x = -1
                self.mouse_position_y = -1

        return QColor(0, 0, 0)
