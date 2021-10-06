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

from PyQt5.QtWidgets import (QGraphicsRectItem, QGraphicsItem, QApplication)
from PyQt5.QtGui import QColor, QPen, QBrush
from PyQt5.QtCore import QSizeF, QRectF, Qt

from enum import Enum, unique

import logging

log = logging.getLogger(__name__)


@unique
class ResizeDirectionHorizontal(Enum):
    HorzNone = 0
    Left = 1
    Right = 2


@unique
class ResizeDirectionVertical(Enum):
    VertNone = 0
    Top = 1
    Bottom = 2


class ResizeDirections(object):

    def __init__(self):
        self.horizontal = ResizeDirectionHorizontal.HorzNone
        self.vertical = ResizeDirectionVertical.VertNone

    def active(self):
        return ((self.horizontal != ResizeDirectionHorizontal.HorzNone) or
                (self.vertical != ResizeDirectionVertical.VertNone))


class ResizeableRectItemSettings(object):
    """
    Settings for a ResizeableRectItem
    """
    def __init__(self,
                 bordersize=15,
                 color=QColor("red"),
                 minimumSize=QSizeF(8.0, 8.0),
                 maximumSize=QSizeF(1000000, 1000000)):

        self.minimumSize = minimumSize
        self.maximumSize = maximumSize
        # width the rect shall use for allowing resize interactions
        self.resizeableBorderSize = bordersize

        self.color = color
        self.pen = QPen(self.color)
        self.pen.setStyle(Qt.DashLine)

        self.color.setAlpha(255/3)
        self.brush = QBrush(self.color)

    def validateRect(self, r: QRectF, resizeDirections):

        left = r.left()
        top = r.top()
        right = left + r.width()
        bottom = top + r.height()

        # The qBound() is used for enforcement of the minimum and maximum sizes.
        # It's derived after solving the following inequalities (example is for
        # left-resizing):

        # minWidth <= newWidth <= maxWidth
        # minWidth <= right-newLeft <= maxWidth
        # minWidth-right <= -newLeft <= maxWidth-right
        # right-minWidth >= newLeft >= right-maxWidth

        # Ditto for the other 3 directions.
        def qBound(minVal, current, maxVal):
            return max(min(current, maxVal), minVal)

        if resizeDirections.horizontal == ResizeDirectionHorizontal.Left:
            left = qBound(right - self.maximumSize.width(), left, right - self.minimumSize.width())
        elif resizeDirections.horizontal == ResizeDirectionHorizontal.Right:
            right = qBound(self.minimumSize.width() + left, right, self.maximumSize.width() + left)

        if resizeDirections.vertical == ResizeDirectionVertical.Top:
            top = qBound(bottom - self.maximumSize.height(), top, bottom - self.minimumSize.height())
        elif resizeDirections.vertical == ResizeDirectionVertical.Bottom:
            bottom = qBound(self.minimumSize.height() + top, bottom, self.maximumSize.height() + top)

        return QRectF(left, top, right - left, bottom - top)


class ResizeableRectItem(QGraphicsRectItem):
    """
    ResizeableRectItem is a QGraphicsItem that displays a transparent overlay.
    The overlay is resizable via mouse interaction at its borders.
    The width of this border is defined through the settings instance.
    """
    def __init__(self, rect: QRectF, settings, parent=None):
        super(ResizeableRectItem, self).__init__(parent)

        self.settings = settings
        self.resizeDirection = ResizeDirections()

        # Horizontal and vertical distance from the cursor position at the time of
        # mouse-click to the nearest respective side of the rectangle. Whether
        # it's left or right, and top or bottom, depends on which side we'll be
        # resizing. We use that to calculate the rectangle from the mouse position
        # during the mouse move events.
        self.horizontalDistance = 0.0
        self.verticalDistance = 0.0
        self.current_cursor = None

        self.setRect(rect)
        self.setFlag(QGraphicsItem.ItemSendsGeometryChanges)
        self.setAcceptHoverEvents(True)

    def __find_resize_direction(self, event):
        """
        Determine horizontal and vertical resize directions by
        comparing the incoming mouse position from event with out rectangle
        """
        innerRect = self.getInnerRect()
        pos = event.pos()

        if pos.x() < innerRect.left():
            self.resizeDirection.horizontal = ResizeDirectionHorizontal.Left
            self.horizontalDistance = self.rect().left() - pos.x()
        elif pos.x() > innerRect.right():
            self.resizeDirection.horizontal = ResizeDirectionHorizontal.Right
            self.horizontalDistance = self.rect().right() - pos.x()
        else:
            self.resizeDirection.horizontal = ResizeDirectionHorizontal.HorzNone

        if pos.y() < innerRect.top():
            self.resizeDirection.vertical = ResizeDirectionVertical.Top
            self.verticalDistance = self.rect().top() - pos.y()
        elif pos.y() > innerRect.bottom():
            self.resizeDirection.vertical = ResizeDirectionVertical.Bottom
            self.verticalDistance = self.rect().bottom() - pos.y()
        else:
            self.resizeDirection.vertical = ResizeDirectionVertical.VertNone

    def __move_to_foreground(self):
        """"""

        items = self.scene().items()

        for item in items[:-1]:
            if item is self:
                continue
            self.stackBefore(item)

    def mousePressEvent(self, event):

        self.__move_to_foreground()

        self.__find_resize_direction(event)

        # If not a resize event, pass it to base class
        # so the move-event can be implemented.
        if not self.resizeDirection.active():
            super().mousePressEvent(event)

    def mouseMoveEvent(self, event):

        if not self.resizeDirection.active():
            super().mouseMoveEvent(event)
        else:
            self.resizeRect(event)

    def mouseReleaseEvent(self, event):

        if not self.resizeDirection.active():
            super().mouseReleaseEvent(event)
        else:
            self.resizeRect(event)

    def hoverEnterEvent(self, event):
        """
        Hover events are used to change to mouse cursor when near the borders
        This is used as an indicator for resizing
        """
        self.__find_resize_direction(event)
        self.adjust_mouse_cursor(event)
        super().hoverEnterEvent(event)

    def hoverMoveEvent(self, event):
        self.__find_resize_direction(event)
        self.adjust_mouse_cursor(event)
        super().hoverMoveEvent(event)

    def hoverLeaveEvent(self, event):
        """
        Restore the previous cursor when leaving this widget
        """
        QApplication.restoreOverrideCursor()
        QApplication.restoreOverrideCursor()

        super().hoverLeaveEvent(event)

    def __set_mouse_cursor(self, cursor):
        """"""

        # pop from cursor stack
        QApplication.restoreOverrideCursor()

        QApplication.setOverrideCursor(cursor)
        self.current_cursor = cursor

    def adjust_mouse_cursor(self, event):
        """

        """

        if self.getInnerRect().contains(event.pos()):
            QApplication.restoreOverrideCursor()
            QApplication.setOverrideCursor(Qt.SizeAllCursor)
        else:
            cursor = Qt.SizeAllCursor

            if self.resizeDirection.horizontal != ResizeDirectionHorizontal.HorzNone:
                cursor = Qt.SizeHorCursor

            if self.resizeDirection.vertical != ResizeDirectionVertical.VertNone:
                cursor = Qt.SizeVerCursor

            if (self.resizeDirection.vertical == ResizeDirectionVertical.Top and
                    self.resizeDirection.horizontal == ResizeDirectionHorizontal.Left):
                cursor = Qt.SizeFDiagCursor

            if (self.resizeDirection.vertical == ResizeDirectionVertical.Bottom and
                    self.resizeDirection.horizontal == ResizeDirectionHorizontal.Right):
                cursor = Qt.SizeFDiagCursor

            if (self.resizeDirection.vertical == ResizeDirectionVertical.Top and
                    self.resizeDirection.horizontal == ResizeDirectionHorizontal.Right):
                cursor = Qt.SizeBDiagCursor

            if (self.resizeDirection.vertical == ResizeDirectionVertical.Bottom and
                    self.resizeDirection.horizontal == ResizeDirectionHorizontal.Left):
                cursor = Qt.SizeBDiagCursor

            self.__set_mouse_cursor(cursor)

    def getInnerRect(self):
        a = self.settings.resizeableBorderSize
        rect = self.rect()
        return rect.adjusted(a, a, -a, -a)

    def resizeRect(self, event):
        left = self.rect().left()
        right = left + self.rect().width()
        top = self.rect().top()
        bottom = top + self.rect().height()

        if self.resizeDirection.horizontal == ResizeDirectionHorizontal.Left:
            left = event.pos().x() + self.horizontalDistance
        elif self.resizeDirection.horizontal == ResizeDirectionHorizontal.Right:
            right = event.pos().x() + self.horizontalDistance

        if self.resizeDirection.vertical == ResizeDirectionVertical.Top:
            top = event.pos().y() + self.verticalDistance
        elif self.resizeDirection.vertical == ResizeDirectionVertical.Bottom:
            bottom = event.pos().y() + self.verticalDistance

        newRect = QRectF(left, top, right - left, bottom - top)
        newRect = self.settings.validateRect(newRect, self.resizeDirection)

        if newRect != self.rect():
            # The documentation states this function should be called prior to any changes
            # in the geometry:
            # Prepares the item for a geometry change. Call this function before
            # changing the bounding rect of an item to keep QGraphicsScene's index
            # up to date.
            self.prepareGeometryChange()

            # For top and left resizing, we move the item's position in the
            # parent. This is because we want any child items it has to move along
            # with it, preserving their distance relative to the top-left corner
            # of the rectangle, because this is the most-expected behavior from a
            # user's point of view.
            # mapToParent() is needed for rotated rectangles.
            self.setPos(self.mapToParent(newRect.topLeft() - self.rect().topLeft()))
            newRect.translate(self.rect().topLeft() - newRect.topLeft())

            self.setRect(newRect)

    def paint(self, painter, option, widget):

        super().paint(painter, option, widget)

        oldPen = painter.pen()
        oldBrush = painter.brush()

        painter.setPen(self.settings.pen)
        painter.setBrush(self.settings.brush)

        painter.drawRect(self.rect())

        painter.setPen(oldPen)
        painter.setBrush(oldBrush)
