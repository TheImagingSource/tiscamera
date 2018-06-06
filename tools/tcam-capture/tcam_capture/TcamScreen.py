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

from tcam_capture.ViewItem import ViewItem

from PyQt5 import QtGui, QtWidgets
from PyQt5.QtWidgets import (QGraphicsView, QGraphicsScene,
                             QGraphicsTextItem, QGraphicsPixmapItem)
from PyQt5.QtCore import pyqtSignal, Qt, QSizeF
from PyQt5.QtGui import QBrush, QColor, QPixmap


class TcamScreen(QtWidgets.QGraphicsView):

    new_pixmap = pyqtSignal(QtGui.QPixmap)
    new_pixel_under_mouse = pyqtSignal(bool, int, int, QtGui.QColor)
    destroy_widget = pyqtSignal()

    def __init__(self, parent=None):
        super(TcamScreen, self).__init__(parent)
        self.setMouseTracking(True)
        self.setSizePolicy(QtWidgets.QSizePolicy.Expanding,
                           QtWidgets.QSizePolicy.Expanding)
        self.setDragMode(QGraphicsView.ScrollHandDrag)
        self.setFrameStyle(0)
        self.scene = QGraphicsScene(self)
        self.setScene(self.scene)

        self.new_pixmap.connect(self.on_new_pixmap)
        self.pix = ViewItem()
        self.scene.addItem(self.pix)
        self.scene.setSceneRect(self.pix.boundingRect())

        self.is_fullscreen = False

        # Flag to differentiate between actual images
        # and 'fake images' i.e. color background + text while
        # waiting for first trigger image
        self.display_real_image = True
        self.text_item = None

        self.mouse_position_x = -1
        self.mouse_position_y = -1

        self.zoom_factor = 1.0
        self.first_image = True

    def reset_zoom(self):

        self.zoom_factor = 1.0
        # this resets the view internal transformation matrix
        self.setTransform(QtGui.QTransform())

    def on_new_pixmap(self, pixmap):
        self.pix.setPixmap(pixmap)

        if not self.display_real_image:
            self.text_item.hide()
            self.scene.removeItem(self.text_item)
            self.display_real_image = True

        if self.first_image:
            self.reset_zoom()
            self.scene.setSceneRect(self.pix.boundingRect())
            self.first_image = False

        self.send_mouse_pixel()
        # don't call repaint here
        # it causes problems once the screen goes blank due to screensavers, etc
        # self.repaint()

    def wait_for_first_image(self):

        if not self.display_real_image:
            return

        self.reset_zoom()
        self.display_real_image = False

        self.text_item = QGraphicsTextItem()
        self.text_item.setDefaultTextColor(QColor("white"))

        self.text_item.setPos(100, 70)
        self.text_item.setPlainText("In Trigger Mode. Waiting for first image...")

        bg = QPixmap(1280, 720)
        bg.fill(QColor("grey"))
        self.pix.setPixmap(bg)
        self.scene.addItem(self.text_item)

    def send_mouse_pixel(self):
        # mouse positions start at 0
        # we want the lower right corner to have the correct coordinates
        # e.g. an 1920x1080 image should have the coordinates
        # 1920x1080 for the last pixel
        self.new_pixel_under_mouse.emit(self.pix.mouse_over,
                                        self.mouse_position_x + 1,
                                        self.mouse_position_y + 1,
                                        self.pix.get_mouse_color())

    def mouseMoveEvent(self, event):
        mouse_position = self.mapToScene(event.pos())
        self.mouse_position_x = mouse_position.x()
        self.mouse_position_y = mouse_position.y()
        super().mouseMoveEvent(event)

    def is_scene_larger_than_image(self):
        """
        checks if the entire ViewItem is visible in the scene
        """
        port_rect = self.viewport().rect()
        scene_rect = self.mapToScene(port_rect).boundingRect()
        item_rect = self.pix.mapRectFromScene(scene_rect)

        isec = item_rect.intersected(self.pix.boundingRect())

        res = self.pix.get_resolution()
        if (isec.size().width() >= QSizeF(res).width() and
                isec.size().height() >= QSizeF(res).height()):
            return True
        return False

    def wheelEvent(self, event):

        if not self.display_real_image:
            return
        # Zoom Factor
        zoomInFactor = 1.25
        zoomOutFactor = 1 / zoomInFactor

        # Set Anchors
        self.setTransformationAnchor(QGraphicsView.NoAnchor)
        self.setResizeAnchor(QGraphicsView.NoAnchor)

        # Save the scene pos
        oldPos = self.mapToScene(event.pos())

        # Zoom
        if event.angleDelta().y() > 0:
            zoomFactor = zoomInFactor
        else:
            zoomFactor = zoomOutFactor

        if (self.is_scene_larger_than_image() and
                zoomFactor < 1.0):
            return

        self.zoom_factor *= zoomFactor

        # we scale the view itself to get infinite zoom
        # so that we can inspect a single pixel
        self.scale(zoomFactor, zoomFactor)

        # Get the new position
        newPos = self.mapToScene(event.pos())

        # Move scene to old position
        delta = newPos - oldPos
        self.translate(delta.x(), delta.y())

        self.scene.setSceneRect(self.pix.boundingRect())

    def set_scale_position(self, scale_factor, x, y):
        self.scale(scale_factor, scale_factor)
        self.translate(x, y)

    def keyPressEvent(self, event):
        if self.isFullScreen():
            if (event.key() == Qt.Key_F11 or
                    event.key() == Qt.Key_Escape or
                    event.key() == Qt.Key_F):
                self.destroy_widget.emit()
        else:
            # ignore event so that parent widgets can use it
            event.ignore()
