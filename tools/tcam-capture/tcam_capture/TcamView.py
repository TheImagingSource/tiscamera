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
import time
from tcam_capture.CapsDesc import CapsDesc
from tcam_capture.TcamScreen import TcamScreen
from tcam_capture.ImageSaver import ImageSaver
from tcam_capture.VideoSaver import VideoSaver
from tcam_capture.TcamCaptureData import TcamCaptureData
from PyQt5 import QtGui, QtWidgets, QtCore
from PyQt5.QtWidgets import (QWidget, QHBoxLayout)
from PyQt5.QtCore import QObject, pyqtSignal, Qt, QEvent

import logging

import gi

gi.require_version("Gst", "1.0")
gi.require_version("Tcam", "0.1")
gi.require_version("GstVideo", "1.0")

from gi.repository import Tcam, Gst, GLib, GstVideo

log = logging.getLogger(__name__)


class TcamView(QWidget):

    image_saved = pyqtSignal(str)
    new_pixel_under_mouse = pyqtSignal(bool, int, int, QtGui.QColor)
    current_fps = pyqtSignal(float)
    format_selected = pyqtSignal(str, str, str)  # format, widthxheight, framerate

    def __init__(self, serial, parent=None):
        super(TcamView, self).__init__(parent)
        self.layout = QHBoxLayout()
        self.container = TcamScreen(self)
        self.container.new_pixel_under_mouse.connect(self.new_pixel_under_mouse_slot)
        self.fullscreen_container = None  # separate widget for fullscreen usage
        self.is_fullscreen = False

        self.layout.addWidget(self.container)
        self.layout.setSizeConstraint(QtWidgets.QLayout.SetMaximumSize)
        self.setLayout(self.layout)
        self.serial = serial
        self.data = TcamCaptureData()
        self.pipeline = None
        self.image = None
        self.mouse_is_pressed = False
        self.current_width = 0
        self.current_height = 0
        self.device_lost_callbacks = []
        self.format_menu = None
        self.caps_desc = None
        self.video_format = None
        self.retry_countdown = 5
        self.actual_fps = 0.0
        self.framecounter = 0
        self.start_time = 0

    def get_caps_desc(self):
        if not self.caps_desc:
            caps = self.get_tcam().get_static_pad("src").query_caps()
            self.caps_desc = CapsDesc(caps)
        return self.caps_desc

    def new_pixel_under_mouse_slot(self, active: bool,
                                   mouse_x: int, mouse_y: int,
                                   color: QtGui.QColor):
        self.new_pixel_under_mouse.emit(active, mouse_x, mouse_y, color)

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

        state = self.pipeline.get_state(1000000).state
        if state == Gst.State.PLAYING or state == Gst.State.PAUSED:
            log.debug("Setting pipeline to READY")
            self.pipeline.set_state(Gst.State.READY)
        self.video_format = video_format
        if video_format is not None:
            log.info("Setting format to {}".format(video_format))
            caps = self.pipeline.get_by_name("bin")
            caps.set_property("device-caps",
                              video_format)
        log.info("Setting state PLAYING")
        self.pipeline.set_state(Gst.State.PLAYING)
        self.start_time = 0
        self.framecounter = 0
        self.container.first_image = True

    def fps_tick(self):
        """
        Recalculate the current fps and emit current_fps signal
        """
        self.framecounter += 1
        if self.start_time == 0:
            self.start_time = time.time()
        else:
            diff = int(time.time() - self.start_time)
            if diff == 0:
                return
            self.actual_fps = self.framecounter / diff
            log.info("{} = {} / {}".format(self.actual_fps,
                                           self.framecounter,
                                           diff))
            self.current_fps.emit(self.actual_fps)

    def new_buffer(self, appsink):
        """
        callback for appsink new-sample signal
        converts gstbuffer into qpixmap and gives it to the display container
        """
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

            self.fps_tick()

            self.image = QtGui.QPixmap.fromImage(QtGui.QImage(buffer_map.data,
                                                              struc.get_value("width"),
                                                              struc.get_value("height"),
                                                              QtGui.QImage.Format_ARGB32))
            if self.fullscreen_container is not None:
                self.fullscreen_container.new_pixmap.emit(self.image)
            else:
                self.container.new_pixmap.emit(self.image)

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
                        "! video/x-raw,format=BGRx "
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
        self.bus.connect('message::error', self.on_error)
        self.bus.connect('message::info', self.on_info)

        self.data.tcam = self.pipeline.get_by_name("bin")

        self.pipeline.set_state(Gst.State.READY)
        log.debug("Created pipeline and set to READY")

    def pause(self):
        log.info("Setting state to PAUSED")
        if self.pipeline:
            self.pipeline.set_state(Gst.State.PAUSED)
        else:
            log.error("Pipeline object does not exist.")
        self.start_time = 0
        self.framecounter = 0

    def stop(self):
        log.info("Setting state to NULL")
        self.start_time = 0
        self.pipeline.set_state(Gst.State.NULL)
        log.info("Set State to NULL")

    def on_info(self, bus, msg):
        info, dbg = msg.parse_info()

        log.info(dbg)

        if msg.src.get_name() == "bin":

            if dbg.startswith("Working with src caps:"):
                log.info("{}".format(dbg.split(": ")[1]))
                self.fire_format_selected(dbg.split(": ")[1])
            else:
                log.error("Info from bin: {}".format(dbg))
        else:
            log.error("ERROR:", msg.src.get_name())  # , ":", info.message)
            if dbg:
                log.debug("Debug info:", dbg)

    def fire_format_selected(self, caps: str):

        c = Gst.Caps.from_string(caps)

        structure = c.get_structure(0)

        if structure.get_name() == "image/jpeg":
            fmt = "jpeg"
        else:
            fmt = structure.get_value("format")

        resolution = "{}x{}".format(structure.get_value("width"),
                                    structure.get_value("height"))

        # compatability problems
        # Older python bindings do not know the type Gst.Fraction.
        # Thus we have to work around this problem...
        results = re.search("framerate=\(fraction\)\d+/\d+", caps)

        if results:
            fps = results.group()
            fps = fps.replace("framerate=(fraction)", "")
        else:
            fps = None
            log.error("Unable to determine framerate settings. This will affect usability.")

        self.format_selected.emit(fmt, resolution, str(fps))

    def on_error(self, bus, msg):
        err, dbg = msg.parse_error()

        if msg.src.get_name() == "tcambin-source":
            if err.message == "Device lost":
                log.error("Received device lost message")
                self.fire_device_lost()
            else:
                log.error("Error from source: {}".format(err.message))

                self.retry_countdown -= 1

                if self.retry_countdown == 0:
                    log.error("Repeatedly retried to start stream. No Success. Giving up.")
                    return

                log.info("Trying restart of stream")
                self.stop()
                self.play(self.video_format)

        else:
            log.error("ERROR: {} : {}".format(msg.src.get_name(), err.message))
            if dbg:
                log.debug("Debug info: {}".format(dbg))

    def on_state_changed(self, bus, msg):
        """
        this function is called when the pipeline changes states.
        we use it to keep track of the current state
        """
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

    def get_tcam(self):
        return self.data.tcam

    def register_device_lost(self, callback):
        self.device_lost_callbacks.append(callback)

    def fire_device_lost(self):
        for cb in self.device_lost_callbacks:
            cb()
