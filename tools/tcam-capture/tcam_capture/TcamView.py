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

import re
import functools
from tcam_capture.ImageSaver import ImageSaver
from tcam_capture.VideoSaver import VideoSaver
from tcam_capture.PropertyWidget import PropertyWidget, Prop
from tcam_capture.TcamSignal import TcamSignals
from tcam_capture.TcamCaptureData import TcamCaptureData
from PyQt5 import QtGui, QtWidgets, QtCore
from PyQt5.QtWidgets import (QApplication, QWidget, QDialog,
                             QHBoxLayout, QVBoxLayout,
                             QAction, QMenu, QGraphicsView,
                             QGraphicsItem, QGraphicsScene, QGraphicsPixmapItem)

from PyQt5.QtCore import QObject, pyqtSignal, Qt, QEvent, QMutex

import logging

import gi

gi.require_version("Gst", "1.0")
gi.require_version("Tcam", "0.1")
gi.require_version("GstVideo", "1.0")

from gi.repository import Tcam, Gst, GLib, GstVideo

log = logging.getLogger(__name__)


class ViewItem(QtWidgets.QGraphicsPixmapItem):
    """Derived class enables mouse tracking for color under mouse retrieval"""
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

    def get_mouse_color(self):
        if self.mouse_over:
            return self.pixmap().toImage().pixelColor(self.mouse_position_x,
                                                      self.mouse_position_y)
        else:
            return QtGui.QColor(0, 0, 0)


class TcamScreen(QtWidgets.QGraphicsView):

    new_pixmap = pyqtSignal(QtGui.QPixmap)
    new_pixel_under_mouse = pyqtSignal(bool, QtGui.QColor)
    destroy_widget = pyqtSignal()

    def __init__(self, parent=None):
        super(TcamScreen, self).__init__(parent)
        self.setMouseTracking(True)
        self.mutex = QMutex()
        self.setDragMode(QGraphicsView.ScrollHandDrag)
        self.setFrameStyle(0)
        self.scene = QGraphicsScene(self)
        self.setScene(self.scene)
        self.new_pixmap.connect(self.on_new_pixmap)
        self.pix = ViewItem()
        self.scene.addItem(self.pix)

        self.factor = 1.0
        self.pix_width = 0
        self.pix_height = 0

        self.orig_parent = None
        self.is_fullscreen = False
        self.scale_factor = 1.0
        self.scene_position_x = None
        self.scene_position_y = None

        self.mouse_position_x = -1
        self.mouse_position_y = -1

    def on_new_pixmap(self, pixmap):
        self.pix.setPixmap(pixmap)
        self.send_mouse_pixel()

    def send_mouse_pixel(self):

        self.new_pixel_under_mouse.emit(self.pix.mouse_over, self.pix.get_mouse_color())

    def mouseMoveEvent(self, event):
        mouse_position = event.pos()
        self.mouse_position_x = mouse_position.x()
        self.mouse_position_y = mouse_position.y()
        super().mouseMoveEvent(event)

    def wheelEvent(self, event):
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
        self.scale(zoomFactor, zoomFactor)

        # Get the new position
        newPos = self.mapToScene(event.pos())

        # Move scene to old position
        delta = newPos - oldPos
        self.scene_position_x = delta.x()
        self.scene_position_y = delta.y()
        self.translate(delta.x(), delta.y())

    def set_scale_position(self, scale_factor, x, y):
        self.scale(scale_factor, scale_factor)
        self.translate(x, y)

    def keyPressEvent(self, event):
        if self.isFullScreen():
            if (event.key() == Qt.Key_F11
                or event.key() == Qt.Key_Escape
                or event.key() == Qt.Key_F):
                self.destroy_widget.emit()
        else:
            # ignore event so that parent widgets can use it
            event.ignore()


class TcamView(QWidget):

    image_saved = pyqtSignal(str)
    new_pixel_under_mouse = pyqtSignal(bool, QtGui.QColor)

    def __init__(self, serial, parent=None):
        super(TcamView, self).__init__(parent)
        self.layout = QHBoxLayout()
        self.container = TcamScreen(self)
        self.container.new_pixel_under_mouse.connect(self.new_pixel_under_mouse_slot)
        self.fullscreen_container = None  # separate widget for fullscreen usage
        self.is_fullscreen = False

        self.layout.addWidget(self.container)
        self.setLayout(self.layout)
        self.serial = serial
        self.data = TcamCaptureData()
        self.pipeline = None
        self.image = None
        self.mouse_is_pressed = False
        self.current_width = 0
        self.current_height = 0

        self.format_list = ["YUV2", "YUY2", "BGR", "BGRx", "GRAY16_LE", "GRAY8"]

        self.format_dict = {
            "GRAY8": QtGui.QImage.Format_Indexed8,
            "Y16": QtGui.QImage.Format_Mono,
            "BGRx": QtGui.QImage.Format_ARGB32,
            "BGR": QtGui.QImage.Format_RGB888
            # "YUY2": QtGui.QImage.Format_
        }

        self.format_menu = None

    def new_pixel_under_mouse_slot(self, active: bool, color: QtGui.QColor):
        self.new_pixel_under_mouse.emit(active, color)

    def eventFilter(self, obj, event):
        """"""
        if event.type == QEvent.KeyPress:
            if event.key() == Qt.Key_F11:
                self.toggle_fullscreen()
                return True

        return QObject.eventFilter(self, obj, event)

    def toggle_fullscreen(self):
        if self.is_fullscreen:
            self.is_fullscreen = False
            self.showNormal()
            self.fullscreen_container.hide()
            self.fullscreen_container.deleteLater()
            self.fullscreen_container = None
        else:
            self.is_fullscreen = True
            self.fullscreen_container = TcamScreen()
            self.fullscreen_container.showFullScreen()
            self.fullscreen_container.show()
            self.fullscreen_container.setFocusPolicy(QtCore.Qt.StrongFocus)
            self.fullscreen_container.installEventFilter(self.fullscreen_container)
            self.fullscreen_container.destroy_widget.connect(self.toggle_fullscreen)

    def save_image(self, image_type: str):
        self.imagesaver.save_image(image_type)

    def image_saved_callback(self, image_path: str):
        self.image_saved.emit(image_path)

    def start_recording_video(self, video_type: str):
        """"""
        self.videosaver.start_recording_video(video_type)

    def stop_recording_video(self):
        """"""
        self.videosaver.stop_recording_video()

    def play(self, video_format=None):
        if self.pipeline is None:
            self.create_pipeline()

        if self.pipeline.get_state(1000000).state == Gst.State.PLAYING:
            log.debug("Setting pipeline to READY")
            self.pipeline.set_state(Gst.State.READY)

        if video_format is not None:
            log.debug("Setting format to {}".format(video_format))
            caps = self.pipeline.get_by_name("caps")
            caps.set_property("caps", Gst.Caps.from_string(video_format))
        self.pipeline.set_state(Gst.State.PLAYING)

    def new_buffer(self, appsink):
        buf = self.pipeline.get_by_name("sink").emit("pull-sample")
        caps = buf.get_caps()
        struc = caps.get_structure(0)
        b = buf.get_buffer()
        try:
            (ret, buffer_map) = b.map(Gst.MapFlags.READ)
            if self.current_width == 0:
                self.current_width = struc.get_value("width")
            if self.current_height == 0:
                self.current_height = struc.get_value("height")

            buffer_format = struc.get_value("format")

            if buffer_format in self.format_dict:
                self.image = QtGui.QPixmap.fromImage(QtGui.QImage(buffer_map.data,
                                                                  struc.get_value("width"),
                                                                  struc.get_value("height"),
                                                                  self.format_dict[buffer_format]))
                if self.fullscreen_container is not None:
                    self.fullscreen_container.new_pixmap.emit(self.image)
                else:
                    self.container.new_pixmap.emit(self.image)
            else:
                log.error("Format {} is not supported. Unable to display.".format(buffer_format))

        finally:
             b.unmap(buffer_map)

        return Gst.FlowReturn.OK

    def create_pipeline(self, video_format=None):

        # the queue element before the sink is important.
        # it allows set_state to work as expected.
        # the sink is synced with our main thread (the display thread).
        # changing the state from out main thread will cause a deadlock,
        # since the remaining buffers can not be displayed because our main thread
        # is currently in set_state
        pipeline_str = ("tcambin serial={serial} name=bin "
                        "! capsfilter name=caps "
                        "! tee name=tee tee. "
                        "! queue "
                        "! videoconvert "
                        "! video/x-raw,format=BGRx "
                        "! appsink name=sink emit-signals=true")

        self.pipeline = None
        self.pipeline = Gst.parse_launch(pipeline_str.format(serial=self.serial))
        self.imagesaver = ImageSaver(self.pipeline, self.serial)
        self.videosaver = VideoSaver(self.pipeline, self.serial)

        sink = self.pipeline.get_by_name("sink")
        sink.connect("new-sample", self.new_buffer)
        # Create bus to get events from GStreamer pipeline
        self.bus = self.pipeline.get_bus()
        self.bus.add_signal_watch()
        self.bus.enable_sync_message_emission()
        self.bus.connect('message::state-changed', self.on_state_changed)

        self.data.tcam = self.pipeline.get_by_name("bin")

        self.pipeline.set_state(Gst.State.READY)
        log.debug("Created pipeline and set to READY")

    def pause(self):
        log.info("Setting state to PAUSED")
        self.pipeline.set_state(Gst.State.PAUSED)

    def stop(self):
        log.info("Setting state to NULL")

        self.pipeline.set_state(Gst.State.NULL)
        log.info("is NULL")

    # this function is called when the pipeline changes states.
    # we use it to keep track of the current state
    def on_state_changed(self, bus, msg):
        old, new, pending = msg.parse_state_changed()
        if not msg.src == self.pipeline.get_by_name("bin"):
            # not from the playbin, ignore
            return
        self.state = new

        if new == Gst.State.PLAYING:
            sink = self.pipeline.get_by_name("bin")
            pad = sink.get_static_pad("src")
            caps = pad.query_caps()
            fmt = caps.get_structure(0)
            if fmt is None:
                log.error("Unable to determine format.")
                return

            width = fmt.get_value("width")
            height = fmt.get_value("height")
            self.container.resize(width, height)

    def get_tcam(self):
        return self.data.tcam

    def create_format_menu(self, parent=None):
        if self.format_menu is not None:
            self.format_menu.clear()
        else:
            self.format_menu = QMenu("Formats",
                                     parent)
        formats = self.get_tcam().get_static_pad("src").query_caps()

        def get_framerates(fmt):
            try:
                rates = fmt.get_value("framerate")
            except TypeError:
                # Workaround for missing GstValueList support in GI
                substr = fmt.to_string()[fmt.to_string().find("framerate="):]
                # try for frame rate lists
                field, values, remain = re.split("{|}", substr, maxsplit=3)
                rates = [x.strip() for x in values.split(",")]
            return rates

        format_dict = {}

        for j in range(formats.get_size()):
            fmt = formats.get_structure(j)
            try:
                format_string = fmt.get_value("format")
                if format_string == "{ RGBx, xRGB, BGRx, xBGR, RGBA, ARGB, BGRA, ABGR }":
                    format_string = "BGRx"
                elif (format_string is None or
                      format_string == "None" or
                      "bayer" in fmt.get_name()):
                    continue

                if format_string not in format_dict:
                    format_dict[format_string] = QMenu(format_string, self)

                width = fmt.get_value("width")
                height = fmt.get_value("height")
                f_str = "{} - {}x{}".format(format_string,
                                            width,
                                            height)
                if "None" in f_str:
                    continue
                if "range" in format_string:
                    continue
                f_menu = format_dict[format_string].addMenu("{}x{}".format(width, height))

            except TypeError as e:
                log.warning("Could not interpret structure. Omitting.")
                continue

            rates = get_framerates(fmt)
            if rates is None:
                continue
            if type(rates) is Gst.FractionRange:
                continue
            for rate in rates:
                rate = str(rate)
                action = QAction(rate, self)
                action.setToolTip("Set format to '{}'".format(f_str + "@" + rate))
                f = "video/x-raw,format={},width={},height={},framerate={}".format(format_string,
                                                                                   width,
                                                                                   height,
                                                                                   rate)
                action.triggered.connect(functools.partial(self.play, f))
                f_menu.addAction(action)

        # do not iterate the dict, but add manually
        # this is neccessary to ensure the order is always correct
        # for key, value in format_dict.items():
        #     self.format_menu.addMenu(value)

        for f in self.format_list:
            try:
                self.format_menu.addMenu(format_dict[f])
            except:
                continue

    def get_format_menu(self, parent=None):
        """Returns a QMenu which endpoints are connected
        to playing a pipeline with the associated format"""

        if self.format_menu is None:
            if parent is None:
                self.create_format_menu(self)
            else:
                self.create_format_menu(parent)
        return self.format_menu
