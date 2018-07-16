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
from tcam_capture.FileNameGenerator import FileNameGenerator
from tcam_capture.MediaSaver import MediaSaver
from tcam_capture.Settings import Settings
from tcam_capture.Encoder import MediaType, get_encoder_dict
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
    video_saved = pyqtSignal(str)
    new_pixel_under_mouse = pyqtSignal(bool, int, int, QtGui.QColor)
    current_fps = pyqtSignal(float)
    format_selected = pyqtSignal(str, str, str)  # format, widthxheight, framerate
    first_image = pyqtSignal()

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
        self.tcam = None
        self.pipeline = None
        self.image = None
        self.mouse_is_pressed = False
        self.use_dutils = True
        self.current_width = 0
        self.current_height = 0
        self.device_lost_callbacks = []
        self.caps_desc = None
        self.video_format = None
        self.retry_countdown = 5
        self.actual_fps = 0.0
        self.framecounter = 0
        self.start_time = 0
        self.settings = None
        self.video_fng = None
        self.image_fng = None

        # additional timer to update actual_fps
        # when no images arrive
        self.fps_timer = QtCore.QTimer()
        self.fps_timer.timeout.connect(self.fps_tick)

        self.file_pattern = ""
        self.file_location = "/tmp"
        self.caps = None
        self.videosaver = None
        self.imagesaver = None

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

    def set_settings(self, new_settings: Settings):
        """
        Update settings of all subclasses
        """
        self.settings = new_settings
        if not self.video_fng:
            self.video_fng = FileNameGenerator(self.serial,
                                               self.settings.video_name)
        else:
            self.video_fng.set_settings(self.settings.video_name)
        self.video_fng.location = self.settings.save_location
        self.video_fng.file_suffix = get_encoder_dict()[self.settings.video_type].file_ending

        if not self.image_fng:
            self.image_fng = FileNameGenerator(self.serial,
                                               self.settings.image_name)
        else:
            self.image_fng.set_settings(self.settings.image_name)
        self.image_fng.location = self.settings.save_location
        self.image_fng.file_suffix = get_encoder_dict()[self.settings.image_type].file_ending

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
            # either show info that we are in trigger mode and still waiting for the first image
            # or show that last image we had. This way we always have something to show to the user
            if self.is_trigger_mode_on() and self.container.first_image:
                self.fullscreen_container.wait_for_first_image()
            else:
                self.fullscreen_container.on_new_pixmap(self.container.pix.pixmap())

    def save_image(self, image_type: str):

        if not self.imagesaver:
            self.imagesaver = MediaSaver(self.serial, self.caps, MediaType.image)
            self.imagesaver.saved.connect(self.image_saved_callback)
            self.imagesaver.error.connect(self.image_error_callback)
        self.image_fng.set_settings(self.settings.image_name)

        fn = self.image_fng.create_file_name("image")

        self.imagesaver.current_filename = fn
        self.imagesaver.save_image(get_encoder_dict()[image_type])

    def image_saved_callback(self, image_path: str):
        """
        SLOT for imagesaver callback for successfull saving
        """
        self.image_saved.emit(image_path)

    def image_error_callback(self, error_msg: str):
        pass

    def video_saved_callback(self, video_path: str):
        """
        SLOT for videosaver callback for successfull saving
        """
        self.video_saved.emit(video_path)

    def start_recording_video(self, video_type: str):
        """

        """
        if self.videosaver:
            log.error("A video recording is already ongoing.")
            return
        self.videosaver = MediaSaver(self.serial, self.caps, MediaType.video)
        self.videosaver.set_encoder(video_type)
        self.videosaver.location = self.file_location

        self.videosaver.current_filename = self.video_fng.create_file_name()
        self.videosaver.saved.connect(self.video_saved_callback)
        self.videosaver.start_recording_video(video_type)

    def stop_recording_video(self):
        """

        """
        if self.videosaver:
            self.videosaver.stop_recording_video()
            self.videosaver = None

    def play(self, video_format=None):

        if self.videosaver:
            self.stop_recording_video()

        if self.pipeline is None:
            self.create_pipeline()

        self.pipeline.set_state(Gst.State.READY)

        if video_format:
            caps_desc = self.get_caps_desc()
            if caps_desc.contains(video_format):
                self.video_format = video_format
            else:
                log.error("Given format caps could not be found in caps descriptions. {}".format(video_format))
                log.error("Falling back to default behavior.")

        if self.video_format is not None:
            log.info("Setting format to {}".format(video_format))
            caps = self.pipeline.get_by_name("bin")
            caps.set_property("device-caps",
                              video_format)

        self.pipeline.set_state(Gst.State.PLAYING)
        self.start_time = 0
        self.framecounter = 0
        self.fps_timer.start(1000)  # 1 second
        self.container.first_image = True
        if self.is_trigger_mode_on():
            self.container.wait_for_first_image()

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
            self.current_fps.emit(self.actual_fps)

    def new_buffer(self, appsink):
        """
        callback for appsink new-sample signal
        converts gstbuffer into qpixmap and gives it to the display container
        """
        buf = self.pipeline.get_by_name("sink").emit("pull-sample")
        caps = buf.get_caps()
        self.caps = caps
        struc = caps.get_structure(0)
        b = buf.get_buffer()
        try:
            (ret, buffer_map) = b.map(Gst.MapFlags.READ)
            if self.current_width == 0:
                self.current_width = struc.get_value("width")
            if self.current_height == 0:
                self.current_height = struc.get_value("height")

            self.fps_tick()

            if self.container.first_image:
                self.first_image.emit()

            self.image = QtGui.QPixmap.fromImage(QtGui.QImage(buffer_map.data,
                                                              struc.get_value("width"),
                                                              struc.get_value("height"),
                                                              QtGui.QImage.Format_ARGB32))
            if self.fullscreen_container is not None:
                self.fullscreen_container.new_pixmap.emit(self.image)
            else:
                self.container.new_pixmap.emit(self.image)
            if self.videosaver and self.videosaver.accept_buffer:
                self.videosaver.feed_image(b)
            if self.imagesaver and self.imagesaver.accept_buffer:
                self.imagesaver.feed_image(b)

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
        pipeline_str = ("tcambin serial={serial} name=bin use-dutils={dutils} "
                        "! video/x-raw,format=BGRx "
                        "! tee name=tee tee. "
                        "! queue max-size-buffers=2 leaky=downstream "
                        "! videoconvert "
                        "! video/x-raw,format=BGRx "
                        "! appsink name=sink emit-signals=true")

        self.pipeline = None
        self.pipeline = Gst.parse_launch(pipeline_str.format(serial=self.serial,
                                                             dutils=self.use_dutils))

        sink = self.pipeline.get_by_name("sink")
        sink.connect("new-sample", self.new_buffer)
        # Create bus to get events from GStreamer pipeline
        self.bus = self.pipeline.get_bus()
        self.bus.add_signal_watch()
        self.bus.enable_sync_message_emission()
        self.bus.connect('message::state-changed', self.on_state_changed)
        self.bus.connect('message::error', self.on_error)
        self.bus.connect('message::info', self.on_info)

        self.tcam = self.pipeline.get_by_name("bin")

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
        self.fps_timer.stop()

    def stop(self):
        """
        Stop playback
        """
        log.info("Setting state to NULL")
        self.start_time = 0
        self.fps_timer.stop()

        self.pipeline.set_state(Gst.State.NULL)
        log.info("Set State to NULL")

    def on_info(self, bus, msg):
        """
        Callback for gst bus info messages
        """
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
        """
        Emit SIGNAL that the pipeline has selected
        src caps and inform listeners what the caps are
        """
        c = Gst.Caps.from_string(caps)

        structure = c.get_structure(0)

        self.image_fng.set_caps(c)
        self.video_fng.set_caps(c)

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
        """
        Callback for gst bus messages
        Receives errors and chooses appropriate actions
        """
        err, dbg = msg.parse_error()

        if msg.src.get_name() == "tcambin-source":

            if err:

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
        return self.tcam

    def register_device_lost(self, callback):
        self.device_lost_callbacks.append(callback)

    def fire_device_lost(self):
        """
        Notify all callback that our device is gone
        """
        for cb in self.device_lost_callbacks:
            cb()

    def is_trigger_mode_on(self):
        if not self.tcam:

            return False

        try:
            (result, value,
             minval, maxval,
             defval, step,
             valuetype,
             flags,
             category, group) = self.tcam.get_tcam_property("Trigger Mode")
        except TypeError as e:
            log.warning("get_tcam_property failed for '{}'".format("Trigger Mode"))
            # log.("get_tcam_property failed for '{}'".format(name))
            return False

        if valuetype == "boolean":
            if value:
                return True
            return False

        elif valuetype == "enum":
            if value == "On":
                return True

        return True

    @staticmethod
    def has_dutils():
        """
        Check to see if the gstreamer module gsttcamdutils is available.
        """

        factory = Gst.ElementFactory.find("tcamdutils")

        if factory:
            return True
        return False
