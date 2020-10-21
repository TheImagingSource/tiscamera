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
from tcam_capture.FPSCounter import FPSCounter
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

    def __init__(self, serial: str, dev_type: str, parent=None):
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
        self.dev_type = dev_type
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
        self.retry_countdown = 0

        self.settings = None
        self.video_fng = None
        self.image_fng = None

        # additional timer to update actual_fps
        # when no images arrive
        self.fps_timer = QtCore.QTimer()
        self.fps_timer.timeout.connect(self.fps_tick)
        self.fps = None
        self.file_pattern = ""
        self.file_location = "/tmp"
        self.caps = None
        self.state = None
        self.videosaver = None
        self.imagesaver = None
        self.window_id = self.container.winId()
        self.displaysink = None

    def get_caps_desc(self):
        """
        Returns a CapsDesc describing the caps of the currently opened device
        Returns None if device is not opened
        """
        if not self.caps_desc:
            tcam = self.get_tcam()
            if not tcam:
                return None
            caps = tcam.get_static_pad("src").query_caps()
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

        self.use_dutils = self.settings.use_dutils
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
            # self.fullscreen_container.deleteLater()
            self.fullscreen_container = None
            self.displaysink.set_window_handle(self.window_id)
        else:
            self.is_fullscreen = True
            self.fullscreen_container = TcamScreen()
            self.fullscreen_container.is_fullscreen = True

            self.fullscreen_container.setAttribute(QtCore.Qt.WA_DeleteOnClose)
            self.fullscreen_container.showFullScreen()
            self.fullscreen_container.show()
            self.container.first_image = True
            self.displaysink.set_window_handle(self.fullscreen_container.winId())

            self.fullscreen_container.setFocusPolicy(QtCore.Qt.StrongFocus)
            self.fullscreen_container.installEventFilter(self.fullscreen_container)
            self.fullscreen_container.destroy_widget.connect(self.toggle_fullscreen)
            # either show info that we are in trigger mode and still waiting for the first image
            # or show that last image we had. This way we always have something to show to the user
            if self.is_trigger_mode_on() and self.container.first_image:
                self.fullscreen_container.wait_for_first_image()
            else:
                self.fullscreen_container.on_new_pixmap(self.container.pix.pixmap())

    def fit_view(self):
        if self.is_fullscreen:
            self.fullscreen_container.fit_in_view.emit()
        else:
            self.container.fit_in_view.emit()

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

    def get_gst_state(self, timeout=5):
        """
        Arguments:
        timeout=5, optional
        """

        if not self.pipeline:
            return None
        return self.pipeline.get_state(timeout).state

    def play(self, video_format=None):

        if self.videosaver:
            self.stop_recording_video()

        if self.pipeline is None:
            self.create_pipeline()

        if self.get_gst_state() == Gst.State.PLAYING:
            log.debug("Setting state to NULL")
            # Set to NULL to ensure that buffers,
            # etc are destroyed.
            # do this by calling stop
            # so that additional steps like fps.stop()
            # are taken
            self.stop()
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

        if self.state and self.settings.apply_property_cache:
            log.info("Property state found.")
            # log.debug("Setting state: ==>{}<==".format(self.state))
            self.tcam.set_property("state", str(self.state))
            self.state = None
        else:
            log.info("No property state to be applied. Starting vanilla camera")

        log.debug("Setting state to PLAYING")
        self.pipeline.set_state(Gst.State.PLAYING)
        self.fps_timer.start(1000)  # 1 second
        self.fps = FPSCounter()
        self.fps.start()
        self.container.first_image = True
        if self.is_trigger_mode_on():
            self.container.wait_for_first_image()

    def fps_tick(self):
        """
        Recalculate the current fps and emit current_fps signal
        """
        self.current_fps.emit(self.fps.get_fps())

    def new_buffer(self, appsink):
        """
        callback for appsink new-sample signal
        converts gstbuffer into qpixmap and gives it to the display container
        """
        self.fps.tick()
        self.fps_tick()

        if self.container.first_image:
            self.first_image.emit()
            self.container.remove_wait_for_fist_image()

        buf = self.pipeline.get_by_name("sink").emit("pull-sample")
        caps = buf.get_caps()
        self.caps = caps
        if (not (self.videosaver and self.videosaver.accept_buffer) and
                not (self.imagesaver and self.imagesaver.accept_buffer)):
            return Gst.FlowReturn.OK

        b = buf.get_buffer()
        if self.videosaver and self.videosaver.accept_buffer:
            self.videosaver.feed_image(b)
        if self.imagesaver and self.imagesaver.accept_buffer:
            self.imagesaver.feed_image(b)

        return Gst.FlowReturn.OK

    def create_pipeline(self, video_format=None):

        # we cheat
        # inject the type into the serial
        # this ensures that no matter what we
        # always have the correct backend
        if self.dev_type:
            self.serial = "{}-{}".format(self.serial, self.dev_type.lower())

        # the queue element before the sink is important.
        # it allows set_state to work as expected.
        # the sink is synced with our main thread (the display thread).
        # changing the state from out main thread will cause a deadlock,
        # since the remaining buffers can not be displayed because our main thread
        # is currently in set_state
        pipeline_str = ("tcambin serial={serial} name=bin use-dutils={dutils} "
                        "! video/x-raw,format=BGRx "
                        "! tee name=tee "
                        "! queue max-size-buffers=2 leaky=downstream "
                        "! video/x-raw,format=BGRx "
                        "! appsink name=sink emit-signals=true sync=false drop=true max-buffers=4 "
                        "tee. "
                        "! queue max-size-buffers=2 leaky=downstream "
                        "! videoconvert "
                        "! xvimagesink double-buffer=true sync=false name=displaysink draw-borders=false")

        self.pipeline = None
        self.pipeline = Gst.parse_launch(pipeline_str.format(serial=self.serial,
                                                             type=self.dev_type.lower(),
                                                             dutils=self.use_dutils))

        self.displaysink = self.pipeline.get_by_name("displaysink")

        sink = self.pipeline.get_by_name("sink")
        sink.connect("new-sample", self.new_buffer)
        # Create bus to get events from GStreamer pipeline
        self.bus = self.pipeline.get_bus()
        self.bus.add_signal_watch()
        self.bus.enable_sync_message_emission()
        self.bus.connect('message::error', self.on_error)
        self.bus.connect('message::info', self.on_info)
        self.bus.connect("sync-message::element", self.on_sync_message)

        self.tcam = self.pipeline.get_by_name("bin")

        if video_format:
            self.tcam.set_property("device-caps", video_format)

        # This ready is required so that get_caps_desc
        # works and does not return ANY
        self.pipeline.set_state(Gst.State.READY)
        log.debug("Created pipeline and set to READY")
        log.debug("Pipeline is: {}".format(pipeline_str.format(serial=self.serial,
                                                               type=self.dev_type.lower(),
                                                               dutils=self.use_dutils)))

    def on_sync_message(self, bus, message):

        structure = message.get_structure()
        if structure is None:
            return
        message_name = structure.get_name()
        if message_name == "prepare-window-handle":

            # "Note that trying to get the drawingarea XID in your on_sync_message() handler
            # will cause a segfault because of threading issues."
            # print 'sinkx_overview win_id: %s (%s)' % (self.gstWindowId, self.video_container.winId())
            assert self.window_id
            message.src.set_window_handle(self.window_id)

    def pause(self):
        log.info("Setting state to PAUSED")
        if self.pipeline:
            self.pipeline.set_state(Gst.State.PAUSED)
        else:
            log.error("Pipeline object does not exist.")

        self.fps_timer.stop()
        if self.fps:
            self.fps.stop()

    def stop(self):
        """
        Stop playback
        """
        log.info("Setting state to NULL")
        self.fps_timer.stop()
        if self.fps:
            self.fps.stop()

        self.pipeline.set_state(Gst.State.NULL)

    def on_info(self, bus, msg):
        """
        Callback for gst bus info messages
        """
        info, dbg = msg.parse_info()

        log.info(dbg)

        if msg.src.get_name() == "bin":

            if dbg.startswith("Working with src caps:"):
                log.info("{}".format(dbg.split(": ")[1]))
                self.caps = dbg.split(": ")[1]
                self.fire_format_selected(dbg.split(": ")[1])
            else:
                log.error("Info from bin: {}".format(dbg))
        else:
            log.error("ERROR:", msg.src.get_name())
            if dbg:
                log.debug("Debug info:", dbg)

    def fire_format_selected(self, caps: str):
        """
        Emit SIGNAL that the pipeline has selected
        src caps and inform listeners what the caps are
        """

        if caps is None or caps == "NULL":
            log.error("Bin returned faulty source caps. Not firiing format_selected")
            return

        c = Gst.Caps.from_string(caps)

        if c.is_empty():
            log.error("Received empty caps. Aborting fire_format_selected")
            return

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

                if "Device lost (" in err.message:

                    m = re.search('Device lost \((.*)\)', err.message)
                    log.error("Received device lost message for {}".format(m.group(1)))

                    self.fire_device_lost()
                else:
                    log.error("Error from source: {}".format(err.message))

                    self.retry_countdown -= 1

                    if self.retry_countdown <= 0:
                        log.error("Repeatedly retried to start stream. No Success. Giving up.")
                        return

                    log.info("Trying restart of stream")
                    self.stop()
                    self.play(self.video_format)

        else:
            log.error("ERROR: {} : {}".format(msg.src.get_name(), err.message))
            if dbg:
                log.debug("Debug info: {}".format(dbg))

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

        names = self.tcam.get_tcam_property_names()
        if "Trigger Mode" not in names:
            return False

        try:
            (result, value,
             minval, maxval,
             defval, step,
             valuetype,
             flags,
             category, group) = self.tcam.get_tcam_property("Trigger Mode")
        except TypeError:
            log.warning("get_tcam_property failed for '{}'".format("Trigger Mode"))
            return False

        if valuetype == "boolean":
            if value:
                return True
            return False

        elif valuetype == "enum":
            if value == "On":
                return True

        return True

    def trigger_image(self):
        """
        Checks if trigger mode is active and try to trigger an image
        """
        if self.is_trigger_mode_on():
            self.tcam.set_tcam_property("Software Trigger", True)

    def start_roi_capture(self, finished_signal):
        """
        Start capturing a ROI and emit finished_signal once the capture is finished
        """
        self.container.start_roi_capture(finished_signal)

    def add_roi(self, roi_widget):
        """
        Add the given roi_widget for permanent display.
        Call remove_roi to undo.
        """
        self.container.add_roi(roi_widget)

    def remove_roi(self, roi_widget):
        """
        Remove roi_widget from display
        """
        self.container.remove_roi(roi_widget)

    def get_state(self):
        """
        Retrieve a json description of the current property settings
        Returns:
        str or None
        """
        if not self.tcam:
            return None

        return self.tcam.get_property("state")

    def load_state(self, state: str):
        """
        Arguments:
        state:
        str containing json descibing the property values
        """
        self.state = state

    @staticmethod
    def has_dutils():
        """
        Check to see if the gstreamer module gsttcamdutils is available.
        """

        factory = Gst.ElementFactory.find("tcamdutils")

        if factory:
            return True
        return False
