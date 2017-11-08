#!/usr/bin/env python3

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


import sys

import gi

# the two lines import the tcam introspection
gi.require_version("Tcam", "0.1")
from gi.repository import Tcam

gi.require_version("Gst", "1.0")
gi.require_version("Gtk", "3.0")

from gi.repository import Gtk, Gst, Gio, GObject, GLib


class TisCameraWindow(Gtk.ApplicationWindow):
    """Main window class for the tiscamera Live Video Example"""

    def __init__(self, serial, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.hb = Gtk.HeaderBar()
        button = Gtk.Button.new_with_mnemonic("_Save")
        button.connect("clicked", lambda x: self.save_image())
        self.hb.pack_start(button)
        button = Gtk.Button.new_with_mnemonic("_Close")
        button.connect("clicked", lambda x: self.close())
        self.hb.pack_end(button)
        self.set_titlebar(self.hb)
        self.set_title("tiscamera Live Video Demo")

        self.pipeline = self.create_pipeline()
        display_widget = self.pipeline.get_by_name("sink").get_property("widget")
        self.add(display_widget)
        self.hb.show_all()
        display_widget.show()
        # If a specific serial number was requested on the command line, the "serial" property of the
        # source element will be set accordingly which will cause the source to operate on the corresponding
        # device.
        src = self.pipeline.get_by_name("src")
        if serial:
            src.set_property("serial", serial)

        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.connect("message::eos", self.on_eos)

        GObject.idle_add(self.start_pipeline)


    def on_eos(self, bus, msg):
        dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK,
                                   "Video stream has ended")
        dialog.format_secondary_text("The video capture device got disconnected")
        dialog.run()
        self.close()

    def start_pipeline(self):
        self.pipeline.set_state(Gst.State.PLAYING)
        src = self.pipeline.get_by_name("src")
        if self.pipeline.get_state(10 * Gst.SECOND)[0] != Gst.StateChangeReturn.SUCCESS:
            serial = src.get_property("serial")
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.CANCEL,
                                       "Failed to start the video stream")
            if not serial:
                dialog.format_secondary_text("No video capture device was found.")
            dialog.set_modal(True)
            dialog.run()
            self.close()
        else:
            serial = src.get_property("serial")
            self.hb.set_subtitle("%s (%s)" % (src.get_device_info(serial)[1], serial))
        return False

    def get_serial(self):
        src = self.pipeline.get_by_name("src")
        if src.get_state(10 * Gst.SECOND) == Gst.State.NULL:
            src.set_state(Gst.State.READY)
        return src.get_property("serial")

    def create_pipeline(self):
        """Build the video capture and display pipeline."""

        # Here the video capture pipeline gets created. Elements that are
        # referenced in other places in the application are given a name, so
        # that they could later be retrieved.
        #
        # The pipeline consists of the following elements:
        #
        # tcambin: This is the main capture element that handles all basic
        #   operations needed to capture video images from The Imaging Source
        #   cameras.
        #
        # queue: The queue is a FIFO buffer element. It is set to a capacity of
        #   2 buffers at maximum to prevent it from filling up indifinitely
        #   should the camera produce video frames faster than the host computer
        #   can handle. The creates a new thread on the downstream side of the
        #   pipeline so that all elements coming after the queue operate in
        #   separate thread.
        #
        # videoconvert: This element converts the videoformat coming from the
        #   camera to match the specification given by the "capsfilter" element
        #   that comes next in the pipeline
        #
        # capsfilter: This element specifies the video format. This example just
        #   specifies a BGRx pixel format which means that we just want a color
        #   image format without any preferences on width, height or framerate.
        #   The tcambin will automatically select the biggest image size
        #   supported by the device and sets the maximum frame rate allowed for
        #   this format. If the camera only supports monochrome formats they get
        #   converted to BGRx by the preceeding 'videoconvert' element.
        #
        # videoconvert: The second videoconvert element in the pipeline converts
        #   the BGRx format to a format understood by the video display element.
        #   Since the gtksink should natively support BGRx, the videoconvert
        #   element will just pass the buffers through without touching them.
        #
        # gtksink: This element displays the incoming video buffers. It also
        #   stores a reference to the last buffer at any time so it could be
        #   saved as a still image
        pipeline = Gst.parse_launch(
            'tcambin name=src ! queue max_size_buffers=2 ! videoconvert ! capsfilter caps="video/x-raw,format=BGRx" ! videoconvert ! gtksink name=sink')

        # Enable the "last-sample" support in the sink. This way the last buffer
        # seen by the display element could be retrieved when saving a still
        # image is requested
        sink = pipeline.get_by_name("sink")
        sink.set_property("enable-last-sample", True)

        return pipeline

    def save_image(self):
        """
        This callback function gets called when the "Save" button gets clicked.

        To save an image file, this function first gets the last video buffer
        from the display sink element. The element needs to have the
        "enable-last-buffer" set to "true" to make this functionality work.

        Then a new GStreamer pipeline is created that encodes the buffer to JPEG
        format and saves the result to a new file.
        """
        sink = self.pipeline.get_by_name("sink")
        sample = sink.get_property("last-sample")
        if not sample:
            dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.ERROR, Gtk.ButtonsType.CANCEL,
                                       "Failed to save image file")
            dialog.format_secondary_text("The application did not receive any video image buffers from the camers")
            dialog.set_modal(True)
            dialog.show()
        dialog = Gtk.FileChooserDialog("Please specify a file name for the video image", self,
                                       Gtk.FileChooserAction.SAVE,
                                       (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                                        Gtk.STOCK_SAVE, Gtk.ResponseType.OK))
        response = dialog.run()
        if response == Gtk.ResponseType.OK:
            filename = dialog.get_filename()
            if not filename.endswith(".jpg"):
                filename += ".jpg"
            buffer = sample.get_buffer()
            pipeline = Gst.parse_launch("appsrc name=src ! videoconvert ! jpegenc ! filesink location=%s" % filename)
            src = pipeline.get_by_name("src")
            src.set_property("caps", sample.get_caps())
            pipeline.set_state(Gst.State.PLAYING)
            src.emit("push-buffer", buffer)
            src.emit("end-of-stream")
            pipeline.get_state(Gst.CLOCK_TIME_NONE)
            pipeline.set_state(Gst.State.NULL)
            pipeline.get_state(Gst.CLOCK_TIME_NONE)
        dialog.destroy()

    def close(self):
        self.pipeline.set_state(Gst.State.NULL)
        self.destroy()


class Application(Gtk.Application):

    def __init__(self, *args, **kwargs):
        super().__init__(*args, application_id="org.theimagingsource.live_video_example",
                         flags=Gio.ApplicationFlags.HANDLES_COMMAND_LINE, **kwargs)
        self.add_main_option("serial", ord("s"), GLib.OptionFlags.NONE, GLib.OptionArg.STRING,
                             "Serial number of device to use", "serial")
        self.windows = {}

    def do_command_line(self, command_line):
        options = command_line.get_options_dict()

        if options.contains("serial") and options["serial"] is not None:
            serial = options["serial"]
            if serial in self.windows:
                self.windows[serial].present()
            else:
                window = TisCameraWindow(serial, application=self)
                window.set_size_request(800, 600)
                window.present()
                self.windows[serial] = window
                self.add_window(window)

        self.activate()

    def do_startup(self):
        Gtk.Application.do_startup(self)
        Gst.init(sys.argv)

    def do_activate(self):
        if not self.windows:
            window = TisCameraWindow(None)
            window.set_size_request(800, 600)
            self.windows[window.get_serial()] = window
            window.present()
            self.add_window(window)
        else:
            self.windows[0].present()


if __name__ == "__main__":
    app = Application()
    app.run(sys.argv)