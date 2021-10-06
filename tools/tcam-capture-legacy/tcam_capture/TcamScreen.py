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
                             QGraphicsTextItem,
                             QRubberBand, QApplication)
from PyQt5.QtCore import pyqtSignal, Qt, QSizeF, QSize, QRect, QPoint
from PyQt5.QtGui import QColor, QPixmap

import logging

log = logging.getLogger(__name__)


class TcamScreen(QtWidgets.QGraphicsView):

    new_pixmap = pyqtSignal(QtGui.QPixmap)
    new_pixel_under_mouse = pyqtSignal(bool, int, int, QtGui.QColor)
    destroy_widget = pyqtSignal()
    fit_in_view = pyqtSignal()

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
        self.fit_in_view.connect(self.fit_view)
        self.pix = ViewItem()
        self.scene.addItem(self.pix)
        self.scene.setSceneRect(self.pix.boundingRect())

        self.is_fullscreen = False

        # Flag to differentiate between actual images
        # and 'fake images' i.e. color background + text while
        # waiting for first trigger image
        self.display_real_image = True
        self.text_item = None

        self.fit_in_view_called = False

        self.mouse_position_x = -1
        self.mouse_position_y = -1

        self.zoom_factor = 1.0
        self.first_image = True
        self.image_counter = 0

        self.capture_roi = False
        self.roi_obj = None
        self.roi_origin = None
        self.roi_widgets = []

        self.selection_area = None
        self.capture_widget = None
        self.origin = None

    def fit_view(self):
        """

        """

        self.reset_zoom()
        self.scene.setSceneRect(self.pix.boundingRect())
        self.scene.update()
        self.fitInView(self.scene.sceneRect(), Qt.KeepAspectRatio)

    def reset_zoom(self):

        self.zoom_factor = 1.0
        # this resets the view internal transformation matrix
        self.setTransform(QtGui.QTransform())

    def on_new_pixmap(self, pixmap):
        self.image_counter += 1
        self.pix.setPixmap(pixmap)

        if not self.display_real_image:
            self.text_item.hide()
            self.scene.removeItem(self.text_item)
            self.display_real_image = True

        if self.image_counter == 1:
            self.resize(self.size())
            self.scene.setSceneRect(self.pix.boundingRect())
            self.update()

            self.reset_zoom()
            self.first_image = False

        # wait for the second image
        # resizeEvents, etc appear before the scene has adjusted
        # to the actual image size. By waiting for the 2. image
        # we circumvent this by having the first image making all
        # adjustments for us. The only scenario where this will
        # cause problems is triggering.
        if self.is_fullscreen and self.image_counter == 2:
            self.fit_view()

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

        bg = QPixmap(640, 480)
        bg.fill(QColor("grey"))
        self.pix.setPixmap(bg)
        self.image_counter += 1
        self.scene.addItem(self.text_item)

    def remove_wait_for_fist_image(self):
        """"""

        if not self.first_image:
            return

        if self.text_item in self.scene.items():
            self.scene.removeItem(self.text_item)
        if self.pix in self.scene.items():
            self.scene.removeItem(self.pix)
        self.first_image = False

    def send_mouse_pixel(self):
        # mouse positions start at 0
        # we want the lower right corner to have the correct coordinates
        # e.g. an 1920x1080 image should have the coordinates
        # 1920x1080 for the last pixel

        self.new_pixel_under_mouse.emit(self.pix.legal_coordinates(self.mouse_position_x,
                                                                   self.mouse_position_y),
                                        self.mouse_position_x + 1,
                                        self.mouse_position_y + 1,
                                        self.pix.get_color_at_position(self.mouse_position_x,
                                                                       self.mouse_position_y))

    def mouseMoveEvent(self, event):
        mouse_position = self.mapToScene(event.pos())
        self.mouse_position_x = mouse_position.x()
        self.mouse_position_y = mouse_position.y()

        if self.selection_area:

            # adjust rect since we want to pull in all directions
            # origin can well be bottom left, thus recalc
            def calc_selection_rect():

                x = min(self.origin.x(), event.pos().x())
                y = min(self.origin.y(), event.pos().y())
                x2 = max(self.origin.x(), event.pos().x())
                y2 = max(self.origin.y(), event.pos().y())

                return QPoint(x, y), QPoint(x2, y2)

            p1, p2 = calc_selection_rect()
            self.selection_area.setGeometry(QRect(p1, p2))

        super().mouseMoveEvent(event)

    def mousePressEvent(self, event):
        """"""

        if self.capture_widget:

            self.selection_area = QRubberBand(QRubberBand.Rectangle, self)
            self.selection_area.setGeometry(QRect(event.pos(), QSize()))
            self.origin = event.pos()
            self.selection_area.show()

        super().mousePressEvent(event)

    def mouseReleaseEvent(self, event):

        if self.capture_widget:
            selectionBBox = self.selection_area.rect()
            rect = QRect(self.selection_area.pos(), selectionBBox.size())
            selectionBBox = self.mapToScene(rect).boundingRect().toRect()
            self.capture_widget.emit(selectionBBox)

            self.selection_area.hide()
            self.selection_area = None
            QApplication.restoreOverrideCursor()

        self.capture_widget = None
        self.selection_area = None

        super().mouseReleaseEvent(event)

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
        elif self.capture_widget and event.key() == Qt.Key_Escape:
            self.abort_roi_capture()
        else:
            # Ignore event so that parent widgets can use it.
            # This is only called when we are not fullscreen.
            # Fullscreen causes us to have no parents.
            event.ignore()

    def start_roi_capture(self, finished_signal):
        """
        Capture a region of interest
        """

        self.capture_widget = finished_signal
        QApplication.setOverrideCursor(Qt.CrossCursor)

    def abort_roi_capture(self):
        """
        Abort the capture of a regoin of interest
        """
        self.capture_widget = None
        self.origin = None

        if self.selection_area:
            self.selection_area.hide()
            self.selection_area = None

        QApplication.restoreOverrideCursor()

    def add_roi(self, roi_widget):
        """
        Add roi_widget to the QGraphicsScene for display
        """
        if not roi_widget:
            return

        self.roi_widgets.append(roi_widget)
        self.scene.addItem(roi_widget)
        roi_widget.show()

    def remove_roi(self, roi_widget):
        """
        Remove given roi widget from the scene
        """
        if not roi_widget:
            return

        roi_widget.hide()
        try:
            self.roi_widgets.remove(roi_widget)
        except ValueError as e:
            # This means the widget is not in the list
            pass
