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

from tcam_capture.ResizeableRectItem import ResizeableRectItemSettings, ResizeableRectItem
from tcam_capture.ROIGroup import ROIGroup

from PyQt5.QtWidgets import (QGraphicsRectItem, QGraphicsItem)
# from PyQt5.QtGui import QColor, QPen, QBrush
# from PyQt5.QtCore import pyqtSignal, Qt, QSizeF, QSize, QRect, QRectF, QPoint, QObject
from PyQt5.QtCore import QRectF


import logging

log = logging.getLogger(__name__)


class ROIRectItem(ResizeableRectItem):
    """
    Wrapper around ResizeableRectItem
    Adds boundary check and property io
    """
    def __init__(self, rect: QRectF, settings, group, parent=None):
        super(ROIRectItem, self).__init__(rect, settings, parent)

        self.group = group
        self.position = None
        self.size = None
        self.accept_changes = True
        self.mouse_pressed = False

    def itemChange(self, change, value):
        """
        Overwrite itemChange function
        Do this to restrict movement of the ROI to always be on top of the image
        """
        if (change == QGraphicsItem.ItemPositionChange and self.scene()
                and not self.resizeDirection.active()):
            # value is the new position.
            newPos = value
            rect = self.scene().sceneRect()

            allowed_position_rect = rect
            if not(allowed_position_rect.contains(self.boundingRect())):

                # Keep the item inside the scene rect.
                newPos.setX(min(rect.right() - self.boundingRect().right(),
                                max(newPos.x(), rect.left())))
                newPos.setY(min(rect.bottom() - self.boundingRect().bottom(),
                                max(newPos.y(), rect.top())))
                return newPos

        return QGraphicsRectItem.itemChange(self, change, value)

    def update_pos(self):

        # Given QRectF scene_rect that represents the rectangle I want drawn...
        self.prepareGeometryChange()

        self.setPos(self.position)  # Set the item's "position" relative to the scene

    def update_rect(self):

        self.prepareGeometryChange()

        # position has to be 0:0
        # we always draw from the origin of our containing rectangle
        # the actual position is always set in the qgraphicsscene
        self.setRect(0, 0,
                     int(self.size.width()),
                     int(self.size.height()))

        self.update_pos()

        # update should not be called with prepareGeometryChange
        # object is already considered 'dirty' and will be redrawn
        # self.update()

    def mousePressEvent(self, event):
        """"""
        self.mouse_pressed = True
        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):
        self.mouse_pressed = False
        super().mouseReleaseEvent(event)

    def mouseMoveEvent(self, event):
        """"""

        def update_position():
            scene_pos = self.scenePos()
            # scene_pos = self.pos()

            left = scene_pos.x()
            top = scene_pos.y()

            # log.info("Moved mouse to position: {} : {}".format(left, top))

            if left < 0:
                left = 0

            if top < 0:
                top = 0

            self.group.set_position(left, top)

        if not self.resizeDirection.active():
            # no active direction thus a ROI movement action
            if self.mouse_pressed:
                update_position()
            super().mouseMoveEvent(event)

        else:

            update_position()
            self.group.set_size(int(self.rect().width()),
                                int(self.rect().height()))

            super().mouseMoveEvent(event)
