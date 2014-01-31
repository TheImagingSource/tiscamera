#
# Copyright 2013-2014 The Imaging Source Europe GmbH
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
#

#
# Example to display a color image with
# The Imaging Source USB CMOS cameras
#

import gst
import gtk

#
# Change the DEVICE and width/height according to your camera
# ( Max width/height are:
#   Dxx 72UC02: 2592x1944
#

DEVICE = "/dev/video0"
# GStreamer is multithreaded so threading needs
# to be initialized to avoid blocking!
gtk.gdk.threads_init()


class App (gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self)

        vbox = gtk.VBox()
        self.add(vbox)
        self.ebox = gtk.EventBox()
        vbox.pack_start(self.ebox)

        hbox = gtk.HBox()
        vbox.pack_start(hbox, False)
        label = gtk.Label("Exposure:")
        hbox.pack_start(label)
        scale = gtk.HScale(gtk.Adjustment(upper=0.3))
        hbox.pack_start(scale)
        scale.connect("value-changed",
                      lambda scale,
                      self:
                      self.source.set_property("exposure",
                                               scale.get_value()),
                      self)
        scale.set_digits(3)

        label = gtk.Label("Gain:")
        hbox.pack_start(label)
        scale = gtk.HScale(gtk.Adjustment(upper=255))
        hbox.pack_start(scale)
        scale.connect("value-changed",
                      lambda scale,
                      self:
                      self.source.set_property("gain",
                                               scale.get_value()),
                      self)
        scale.set_digits(0)

        hbox = gtk.HBox()
        vbox.pack_start(hbox, False)
        label = gtk.Label("R:")
        hbox.pack_start(label)
        scale = gtk.HScale(gtk.Adjustment(upper=1024))
        hbox.pack_start(scale)
        scale.connect("value-changed",
                      lambda scale,
                      self:
                      self.source.set_property("red-gain",
                                               int(scale.get_value())),
                      self)
        scale.set_digits(0)

        label = gtk.Label("G:")
        hbox.pack_start(label)
        scale = gtk.HScale(gtk.Adjustment(upper=1024))
        hbox.pack_start(scale)
        scale.connect("value-changed",
                      lambda scale,
                      self:
                      self.source.set_property("green-gain",
                                               int(scale.get_value())),
                      self)
        scale.set_digits(0)

        label = gtk.Label("B:")
        hbox.pack_start(label)
        scale = gtk.HScale(gtk.Adjustment(upper=1024))
        hbox.pack_start(scale)
        scale.connect("value-changed",
                      lambda scale,
                      self:
                      self.source.set_property("blue-gain",
                                               int(scale.get_value())),
                      self)
        scale.set_digits(0)

        button = gtk.ToggleButton(label="Auto")
        hbox.pack_start(button)
        button.connect("toggled",
                       lambda button,
                       self:
                       self.source.set_property("auto_wb",
                                                button.get_active()),
                       self)
        self.set_size_request(800, 600)

        vbox.show_all()
        self.ebox.realize()

    def on_new_buffer(self, sink, buffer, pad, src):
        structure = buffer.get_caps().get_structure(0)
        if buffer.size != (structure["width"] * structure["height"]):
            return
        bayer_caps = gst.Caps("video/x-raw-bayer,"
                              "format=grbg,"
                              "width=%d,"
                              "height=%d"
                              % (structure["width"],
                                 structure["height"]))
        buffer.set_caps(bayer_caps)
        src.emit("push-buffer", buffer)

    def create_pipelines(self):
        self.pipeline = gst.parse_launch('tiscamerasrc device=%s name=source'
                                         '! video/x-raw-rgb '
                                         '! queue '
                                         '! cogcolorspace '
                                         '! xvimagesink'
                                         % (DEVICE,))
        self.source = self.pipeline.get_by_name("source")
        bus = self.pipeline.get_bus()
        bus.add_signal_watch()
        bus.enable_sync_message_emission()
        bus.connect('sync-message::element', self.on_message, self.ebox)

    def start(self):
        print "Starting video pipeline"
        self.pipeline.set_state(gst.STATE_PLAYING)

    def on_message(self, bus, message, widget):
        print "message %s" % message.structure.get_name()
        if message.structure.get_name() == 'prepare-xwindow-id':
            print "set xid"
            gtk.gdk.threads_enter()
            gtk.gdk.display_get_default().sync()
            imagesink = message.src
            imagesink.set_xwindow_id(widget.window.xid)
            gtk.gdk.threads_leave()


if __name__ == "__main__":
    app = App()
    app.show()
    app.create_pipelines()
    app.start()

    gtk.main()
