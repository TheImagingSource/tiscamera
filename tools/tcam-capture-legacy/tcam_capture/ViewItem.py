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

    def get_resolution(self):
        return self.pixmap().size()

    def legal_coordinates(self, x_pos, y_pos):
        if(0 <= x_pos <= self.pixmap().width() and
           0 <= y_pos <= self.pixmap().height()):
            return True
        else:
            return False

    def get_color_at_position(self, x_pos, y_pos):
        """
        Coordinates must be in object coordinates
        """
        if self.legal_coordinates(x_pos, y_pos):
            return QColor(self.pixmap().toImage().pixel(x_pos,
                                                        y_pos))
        else:
            return QColor(0, 0, 0)
