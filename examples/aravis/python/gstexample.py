#
# Copyright 2013 The Imaging Source Europe GmbH
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
# GigE cameras
#

# Unfortunately introspection is buggy when working with gstreamer 0.10.
# The stream might appear in a separate window.


import gi

gi.require_version('Gst', '0.10')

from fractions import Fraction
import math

from gi.repository import Aravis, Gst, Gtk, GObject, Gdk, GdkX11

#
# Change the CAMERA and width/height according to your camera
#

ABSVAL_SLIDER_TICKS = 100

COLOR = "video/x-raw-bayer"
MONO = "video/x-raw-gray"


class Camera:
    """Camera representation containing everythin we need"""
    name = "The Imaging Source Europe GmbH-46210199"  # name used by aravis
    width = 2592  # width of our image
    height = 1944  # height of our image
    pattern = "rggb"  # bayer pattern; only needed for color cameras
    bpp = 8
    depth = 8
    init_fps = 15     # fps we want to start with


orig_pack_start = Gtk.Box.pack_start


def pack_start(self, child, expand=True, fill=True, padding=0):
    orig_pack_start(self, child, expand, fill, padding)

Gtk.Box.pack_start = pack_start


class CamApp(Gtk.Window):
    """Main application class
    Contains gstreamer pipeline and Gtk window"""

    def __init__(self):
        """ Constructor - defines elementsfor the gtk gui"""
        # install our gstreamer pipeline
        self.create_pipelines()

        # we are a Gtk window
        Gtk.Window.__init__(self)

        vbox = Gtk.VBox()
        self.add(vbox)
        self.video_area = Gtk.DrawingArea()  # this will serve as output area
        vbox.pack_start(self.video_area)

        hbox = Gtk.HBox()
        vbox.pack_start(hbox, False)

        # exposure
        label = Gtk.Label("Exposure:")
        hbox.pack_start(label)
        upper = 100

        # retrieve value from camera
        val = self.camera.get_exposure_time()

        # rmin = 0.0
        rmax = 100

        # we transform the actual value
        # so that we are able to create a logarithmic scale

        # the actual formulas are:
        # rangelen2 = math.log(rmax) - math.log(rmin)
        # exposure_value = upper / rangelen2 * (math.log(val) - math.log(rmin))
        # since our minimum is 0, we hardcode this to avoid math domain errors
        # through math.log(0)

        rangelen2 = math.log(rmax)
        exposure_value = upper / rangelen2 * math.log(val)

        self.exp_range = Gtk.Adjustment(exposure_value, 0, upper, 0.1, 1)

        self.exposure_scale = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL,
                                        adjustment=self.exp_range)
        hbox.pack_start(self.exposure_scale)
        self.exposure_scale.connect("value-changed", self.change_exposure)
        self.exposure_scale.set_digits(0)
        # don't draw value; we do that ourself
        self.exposure_scale.set_draw_value(False)
        self.exposure_label = Gtk.Label("0.0")
        hbox.pack_start(self.exposure_label)

        # gain
        label = Gtk.Label("Gain:")
        hbox.pack_start(label)

        # retrieve gain boundaries as tupel
        gain_bounds = self.camera.get_gain_bounds()
        gain_min = gain_bounds[0]
        gain_max = gain_bounds[1]

        gain_range = Gtk.Adjustment(self.camera.get_gain(),
                                    gain_min,
                                    gain_max,
                                    0.1,
                                    1)

        scale = Gtk.Scale(orientation=Gtk.Orientation.HORIZONTAL,
                          adjustment=gain_range)
        hbox.pack_start(scale)
        scale.connect("value-changed", lambda scale, self:
                      self.source.set_property("gain",
                                               scale.get_value()), self)
        scale.set_digits(0)

        # framerate
        button = Gtk.Button("Set Framerate:")
        hbox.pack_start(button)
        self.frame_entry = Gtk.Entry()
        self.frame_entry.set_text("")
        hbox.pack_start(self.frame_entry)
        button.connect("clicked", self.change_framerate)

        self.set_size_request(800, 600)
        self.set_title("Aravis Gstreamer Example")
        # kill application when user closes window
        self.connect('delete-event', Gtk.main_quit)
        vbox.show_all()

    def change_framerate(self, event):
        """Changes the framerate of camera"""
        # empty values should not be persued
        val = self.frame_entry.get_text()
        if val == '':
            print "Value for framerate not valid."
            return
        # set state so that elements can re-negotiate
        self.pipeline.set_state(Gst.State.READY)

        # Retrieve framerate as a fraction,
        # so that we can the caps in a way aravis understands
        new_frame_rate = Fraction(val)

        # Width and height have to be mentioned or
        # aravissrc will encoounter undefined behavior.
        new_caps = Gst.Caps.from_string(("{0}, "
                                         "width={1}, "
                                         "height={2}, "
                                         "framerate={3}/{4}, "
                                         "format={5} ")
                                        .format(COLOR,
                                                Camera.width,
                                                Camera.height,
                                                new_frame_rate.numerator,
                                                new_frame_rate.denominator,
                                                Camera.pattern))

        self.caps.set_property("caps", new_caps)
        self.pipeline.set_state(Gst.State.PLAYING)  # start playing again

    def change_exposure(self, event):
        """Uses received value from scale as value for exposure manipulation"""
        pos = self.exposure_scale.get_value()

        # retrieve the exposure bounds as a tuple
        bounds = self.camera.get_exposure_time_bounds()

        rmin = bounds[0]
        rmax = bounds[1]

        if(math.isnan(pos)):
            pos = 1.0

        if(rmin != 0):
            min_log = math.log(rmin)
        else:
            min_log = 0

        rangelen = math.log(rmax) - min_log
        val = math.exp(min_log + rangelen / ABSVAL_SLIDER_TICKS * pos)

        # stay within boundaries
        if(val > rmax):
            val = rmax
        if(val < rmin):
            val = rmin
        # always go through source element to assure
        # that everybody has the new value
        self.source.set_property("exposure", val)
        self.exposure_label.set_text(str(val))

    def create_pipelines(self):
        """Defines the gstreamer"""
        self.pipeline = Gst.Pipeline()

        self.source = Gst.ElementFactory.make("aravissrc", "source")
        self.caps = Gst.ElementFactory.make("capsfilter", "caps")
        self.queue1 = Gst.ElementFactory.make("queue", "queue1")
        self.bayer = Gst.ElementFactory.make("bayer2rgb", "bayer")
        self.queue2 = Gst.ElementFactory.make("queue", "queue2")

        self.color = Gst.ElementFactory.make("ffmpegcolorspace", "colorspace")
        self.sink = Gst.ElementFactory.make("xvimagesink", "sink")

        # property definitions
        self.source.set_property("camera-name", Camera.name)

        # we retrieve the ArvCamera
        self.camera = self.source.get_property("camera")

        # Set the wished initial settings for capsfilter.
        caps_string = ("video/x-raw-bayer, "
                       "format={0}, "
                       "bpp={1}, "
                       "depth={2}, "
                       "width={3}, "
                       "height={4}, "
                       "framerate={5}/1"
                       .format(Camera.pattern,
                               Camera.bpp,
                               Camera.depth,
                               Camera.width,
                               Camera.height,
                               Camera.init_fps))
        caps = Gst.Caps.from_string(caps_string)
        self.caps.set_property("caps", caps)

        # add elements to pipeline
        self.pipeline.add(self.source)
        self.pipeline.add(self.caps)
        self.pipeline.add(self.queue1)
        self.pipeline.add(self.bayer)
        self.pipeline.add(self.queue2)
        self.pipeline.add(self.color)
        self.pipeline.add(self.sink)

        # define order in which elemts shall interact
        self.source.link(self.caps)
        self.caps.link(self.queue1)
        self.queue1.link(self.bayer)
        self.bayer.link(self.queue2)
        self.queue2.link(self.color)
        self.color.link(self.sink)

        self.bus = self.pipeline.get_bus()
        self.bus.add_signal_watch()
        self.bus.enable_sync_message_emission()

    def start(self):
        """ Changes the pipeline status to GST_STATE_PLAYING"""
        print "Starting video stream"
        # self.sink.set_xwindow_id(self.video_area.window.xid)
        # self.sink.set_window_handle(self.video_area.window.xid)
        self.pipeline.set_state(Gst.State.PLAYING)

    def stop(self):
        """ Changes the pipeline status to GST_STATE_PLAYING"""
        self.pipeline.set_state(Gst.State.READY)


# actual entry point
if __name__ == "__main__":
    # Gstreamer needs to be initialized to avoid blocking!
    Gdk.threads_init()
    GObject.threads_init()
    Gst.init(None)

    app = CamApp()  # define window
    app.show()      # show window

    app.start()     # start playing

    # switch to Gtk main loop and wait for user interaction
    Gtk.main()
