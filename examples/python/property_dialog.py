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
import math

import gi

gi.require_version("Tcam", "0.1")
from gi.repository import Tcam

gi.require_version("Gst", "1.0")
gi.require_version("Gtk", "3.0")
from gi.repository import Gtk, Gst, Gio, GObject, GLib

from collections import namedtuple

from live_video import TisCameraWindow

CameraProperty = namedtuple("CameraProperty",
                            "status value min max default step type flags category group")

class PropertyBox(Gtk.Box):
    def __init__(self, szgroup, name):
        super().__init__(orientation=Gtk.Orientation.HORIZONTAL)
        label = Gtk.Label(name)
        szgroup.add_widget(label)
        self.add(label)


class BooleanProperty(PropertyBox):
    """Base class for a bool type property. Uses a Gtk.Switch to control the value."""
    __gsignals__ = {"changed": (GObject.SIGNAL_RUN_FIRST, None, (str, bool))}
    def __init__(self, szgroup, name, ppty):
        super().__init__(szgroup, name)
        button = Gtk.Switch()
        self.pack_start(button, False, False, 10)
        button.set_active(ppty.value)
        button.connect("notify::active",
                       lambda switch, x, self: self.emit("changed", name, switch.get_active()), self)


class ScaleProperty(PropertyBox):
    """Base class for a scale control. valuetype could be "float" or "int".
    The control will use a logarithmic scale if the range of possible values
    would exceed 5000.
    """
    ABSVAL_SLIDER_TICKS = 100
    def __init__(self, szgroup, name, ppty, valuetype):
        super().__init__(szgroup, name)
        range = ppty.max - ppty.min
        make_logarithmic = range > 5000
        if make_logarithmic:
            if ppty.min > 0:
                minval = math.log(ppty.min)
            else:
                minval = 0
            logrange = math.log(ppty.max) - minval
            if ppty.value > 0:
                logval = math.log(ppty.value)
            else:
                logval = 0
            val = self.ABSVAL_SLIDER_TICKS / logrange * (logval - minval)
            scale = Gtk.Scale.new_with_range(Gtk.Orientation.HORIZONTAL, 0, self.ABSVAL_SLIDER_TICKS, 1)
            scale.set_value(val)
            # Add a mark for the default value
            if ppty.default > 0:
                default = self.ABSVAL_SLIDER_TICKS / logrange * (math.log(ppty.default) - minval)
                scale.add_mark(default, Gtk.PositionType.TOP, None)
            scale.connect("value-changed", self.on_logarithmic_value_changed, name, ppty, valuetype)
        else:
            step = ppty.step or 1.0
            scale = Gtk.Scale.new_with_range(Gtk.Orientation.HORIZONTAL, ppty.min, ppty.max, step)
            scale.set_value(ppty.value)
            # Add a mark for the default value
            scale.add_mark(ppty.default, Gtk.PositionType.TOP, None)
            scale.connect("value-changed", lambda scale: self.emit("changed", name, valuetype(scale.get_value())))
        self.pack_start(scale, True, True, 0)

    def on_logarithmic_value_changed(self, scale, name, ppty, valuetype):
        pos = scale.get_value()

        minval = ppty.min
        if minval > 0:
            minval = math.log(minval)
        else:
            minval = 0
        logrange = math.log(ppty.max) - minval
        val = math.exp(minval + logrange / self.ABSVAL_SLIDER_TICKS * pos)

        # clip to range
        val = max(ppty.min, val)
        val = min(ppty.max, val)

        self.emit("changed", name, valuetype(val))

class IntegerProperty(ScaleProperty):
    __gsignals__ = {"changed": (GObject.SIGNAL_RUN_FIRST, None, (str, int))}
    def __init__(self, szgroup, name, ppty):
        super().__init__(szgroup, name, ppty, int)

class DoubleProperty(ScaleProperty):
    __gsignals__ = {"changed": (GObject.SIGNAL_RUN_FIRST, None, (str, float))}
    def __init__(self, szgroup, name, ppty):
        super().__init__(szgroup, name, ppty, float)


class EnumProperty(PropertyBox):
    """Base class for enum type properties."""
    __gsignals__ = {"changed": (GObject.SIGNAL_RUN_FIRST, None, (str, str))}
    def __init__(self, szgroup, name, ppty, value_list):
        super().__init__(szgroup, name)
        combo_box = Gtk.ComboBoxText()
        for value in value_list:
            combo_box.append_text(value)
        if ppty.value in value_list:
            combo_box.set_active(value_list.index(value))
        combo_box.connect("changed",
                          lambda combo: self.emit("changed", name, combo.get_active_text(), name))
        self.add(combo_box)

class ButtonProperty(Gtk.Button):
    """Base class for button type properties."""
    __gsignals__ = {"changed": (GObject.SIGNAL_RUN_FIRST, None, (str, bool))}
    def __init__(self, szgroup, name):
        super().__init__(name)
        self.connect("clicked", lambda x: self.emit("changed", name, True))


class PropertyDialog(Gtk.Window):
    def __init__(self, src):
        super().__init__()
        self.src = src
        devinfo = src.get_device_info(src.get_property("serial"))
        self.set_title("%s (%s) - Properties" % (devinfo[1], src.get_property("serial")))
        self.add(self.create_controls())
        self.connect("delete-event", self.on_destroy)

    def on_destroy(self, *x):
        self.hide()
        return True

    def create_controls(self):
        """Create a Gtk.Notebook containing the property categories as pages
        and the properties as controls on each page."""
        names = self.src.get_tcam_property_names()
        group = Gtk.SizeGroup()
        group.set_mode(Gtk.SizeGroupMode.HORIZONTAL)
        categories ={}
        for name in names:
            ptype = self.src.get_tcam_property_type(name)
            ppty = CameraProperty(*self.src.get_tcam_property(name))
            if ppty.category not in categories:
                categories[ppty.category] = []
            box = None
            if ptype == "boolean":
                box = BooleanProperty(group, name, ppty)
            elif ptype == "integer":
                if ppty.max <= ppty.min:
                    continue
                box = IntegerProperty(group, name, ppty)
            elif ptype == "double":
                if ppty.max <= ppty.min:
                    continue
                box = DoubleProperty(group, name, ppty)
            elif ptype == "string":
                print('not used')
            elif ptype == "enum":
                entry_list = self.src.get_tcam_menu_entries(name)
                box = EnumProperty(group, name, ppty, entry_list)
            elif ptype == "button":
                box = ButtonProperty(group, name)
            else:
                print("Unknown property type '{}' for property '{}'".format(ptype, name))
            if box:
                categories[ppty.category].append(box)
                box.connect("changed", lambda x, name, value, src: src.set_tcam_property(name, value), self.src)

        notebook = Gtk.Notebook()
        for category in categories:
            page = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
            page.set_border_width(10)
            for box in categories[category]:
                page.add(box)
            notebook.append_page(page, Gtk.Label(category))
        notebook.show_all()
        return notebook


class PropertyCameraWindow(TisCameraWindow):
    """Main application window. Extends the live_video example window with a button to open a property dialog."""
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        button = Gtk.Button.new_with_mnemonic("_Properties")
        self.hb.pack_start(button)
        self.dialog = self.create_dialog()
        button.connect("clicked", lambda x, self: self.dialog.present(), self)
        button.show()

    def create_dialog(self):
        src = self.pipeline.get_by_name("src")
        window = PropertyDialog(src)
        window.set_default_size(600,-1)
        return window


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
                window = PropertyCameraWindow(serial, application=self)
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
            window = PropertyCameraWindow(None)
            window.set_size_request(800, 600)
            self.windows[window.get_serial()] = window
            window.present()
            self.add_window(window)
        else:
            self.windows[0].present()


if __name__ == "__main__":
    app = Application()
    app.run(sys.argv)