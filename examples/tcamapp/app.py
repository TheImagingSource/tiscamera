import gi

gi.require_version ("Gtk", "3.0")
gi.require_version ("Gst", "1.0")
gi.require_version ("Tcam", "0.1")
gi.require_version ("GdkX11", "3.0")
gi.require_version ("GstVideo", "1.0")

from gi.repository import GdkX11, Gtk, Tcam, GstVideo, Gst, GdkPixbuf, GObject

import sys
import re


class DeviceDialog (Gtk.Dialog):
    def __init__(self, parent=None):
        Gtk.Dialog.__init__(self, parent=parent, title="Device Selection")
        self.add_buttons (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                          Gtk.STOCK_OK, Gtk.ResponseType.OK)

        model = Gtk.ListStore(GdkPixbuf.Pixbuf, str, str)
        self.__iv = Gtk.IconView.new_with_model(model)
        self.__iv.set_pixbuf_column(0)
        self.__iv.set_text_column(1)
        self.__iv.set_selection_mode (Gtk.SelectionMode.BROWSE)

        self.source = Gst.ElementFactory.make("tcamsrc")
        if not self.source:
            raise (RuntimeError, "Failed to create tcamsrc element")

        for dev in self.__get_devices(self.source):
            pixbuf = Gtk.IconTheme.get_default().load_icon (
                Gtk.STOCK_YES, 64, 0)
            label = "%s (%s)" % (dev[1], dev[0])
            model.append ((pixbuf, label, dev[0]))
        self.__iv.select_path(Gtk.TreePath(0))
        self.get_content_area().add(self.__iv)

    def __get_devices(self, elem):
        elem.set_property("serial", None)
        ret = []
        for serial in elem.get_device_serials():
            result, name, ident, conn_type = elem.get_device_info(serial)
            ret.append( (serial, name, ident, conn_type))
        return ret

    def get_serial(self):
        return self.__iv.get_model()[self.__iv.get_selected_items()[0]][2]

    def get_source(self):
        if self.__iv.get_selected_items():
            self.source.set_property("serial", self.__iv.get_model()[self.__iv.get_selected_items()[0]][2])
        return self.source

class PropertyDialog (Gtk.Dialog):
    def __init__ (self, src):
        Gtk.Dialog.__init__(self, title="Properties")
        self.set_default_size (300,200)
        self.src = src

        vbox = self.__create_main_vbox ()
        self.get_content_area().add (vbox)
        vbox.show_all()

    def __on_set_property_range(self, scale, name):
        self.src.set_tcam_property (name,
                                    GObject.Value(int,int(scale.get_value())))

    def __on_set_property_toggle(self, toggle, name):
        self.src.set_tcam_property (name,
                                    GObject.Value(bool,
                                                  bool(toggle.get_active())))
    def __on_set_property_button(self, button, name):
        self.src.set_tcam_property (name, GObject.Value(bool,True))

    def __create_main_vbox (self):
        main_vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        nb = Gtk.Notebook()
        main_vbox.pack_start (nb, True, True, 6)
        i = 0
        pagenr = 1
        controls_per_page = 5
        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
        for name in self.src.get_tcam_property_names():
            (result, value, minval, maxval,
             defval, step, valuetype) = self.src.get_tcam_property (name)
            pptytype = self.src.get_tcam_property_type (name)

            if pptytype == "integer":
                ctrl = self.__create_range_control (
                    name, minval, maxval, defval, value, step)
            elif pptytype == "boolean":
                ctrl = self.__create_toggle_control (name, value)
            elif pptytype == "button":
                ctrl = self.__create_button_control (name)
            vbox.pack_start (ctrl, True, False, 6)
            i += 1
            if i > controls_per_page:
                nb.append_page (vbox, Gtk.Label ("%d" % pagenr))
                vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL)
                i = 0
                pagenr += 1
        if i:
            nb.append_page (vbox, Gtk.Label ("%d" % pagenr))

        return main_vbox

    def __create_control_vbox (self, name):
        vbox = Gtk.Box(orientation = Gtk.Orientation.HORIZONTAL)
        label = Gtk.Label (name)
        vbox.pack_start (label, True, False, 2)
        return vbox

    def __create_range_control (self,
                                name, minval, maxval, defval, curval, step):
        vbox = self.__create_control_vbox (name)
        scale = Gtk.Scale.new_with_range (Gtk.Orientation.HORIZONTAL,
                                          minval, maxval, step)
        scale.set_value (curval)
        scale.add_mark (defval, Gtk.PositionType.TOP, None)
        scale.connect ("value-changed", self.__on_set_property_range, name)
        vbox.pack_start (scale, True, True, 2)
        return vbox

    def __create_toggle_control (self, name, defval):
        vbox = self.__create_control_vbox (name)
        button = Gtk.ToggleButton.new_with_label (name)
        button.set_active (defval)
        button.connect ("toggled", self.__on_set_property_toggle, name)
        vbox.pack_start (button, True, False, 2)
        return vbox

    def __create_button_control (self, name):
        button = Gtk.Button(label=name)
        button.connect ("clicked", self.__on_set_property_button, name)
        return button


class AppWindow (Gtk.Window):
    def __init__ (self, source):
        Gtk.Window.__init__(self)

        self.source = source
        self.pipeline = None
        self.ppty_dialog = PropertyDialog(self.source)
        self.ppty_dialog.present()

        self.set_title ("TCam Demo Applikation")
        self.connect ("destroy", Gtk.main_quit)

        hb = Gtk.HeaderBar()
        hb.set_show_close_button (True)
        hb.props.title = ("TCam Demo Applikation")
        self.set_titlebar (hb)
        combo = self.create_format_combo()
        combo.connect ("changed", self.on_format_combo_changed)
        hb.pack_start (combo)
        self.fps_combo = self.create_fps_combo ()
        self.fps_combo.connect ("changed", self.on_fps_combo_changed)
        hb.pack_start (self.fps_combo)
        hb.show_all()

        vbox = Gtk.Box(orientation = Gtk.Orientation.VERTICAL)
        self.add(vbox)

        self.da = Gtk.DrawingArea()
        self.da.set_size_request (640, 480)
        self.da.set_double_buffered (True)
        vbox.pack_start (self.da, True, True, 0)

        vbox.show_all()
        self.da.realize()

        # This selects the default video format and thus
        # starts the live preview
        combo.set_active(0)


    def create_format_combo (self):
        formats = self.get_format_list()
        model = Gtk.ListStore (str, int, GObject.TYPE_PYOBJECT)
        for fmt in formats:
            model.append (fmt)
        combo = Gtk.ComboBox.new_with_model (model)
        renderer_text = Gtk.CellRendererText()
        combo.pack_start (renderer_text, True)
        combo.add_attribute (renderer_text, "text", 0)
        return combo

    def create_fps_combo (self):
        combo = Gtk.ComboBox()
        renderer_text = Gtk.CellRendererText()
        combo.pack_start (renderer_text, True)
        combo.add_attribute (renderer_text, "text", 0)
        return combo

    def on_format_combo_changed (self, combo):
        it = combo.get_active_iter()
        if it != None:
            model = combo.get_model()
            fmt = model[it][1]
            fps = model[it][2]

            fps_model = Gtk.ListStore (str, str, int)
            for rate in fps:
                fps_model.append ((rate[0], rate[1], fmt))
            self.fps_combo.set_model (fps_model)
            self.fps_combo.set_active (0)

    def on_fps_combo_changed (self, combo):
        if self.pipeline:
            self.pipeline.set_state(Gst.State.NULL)
            self.pipeline.get_state(0)
            self.source.unparent()

        it = combo.get_active_iter()
        if it != None:
            model = combo.get_model()
            rate = model[it][1]
            fmt = model[it][2]
            self.pipeline = self.create_pipeline(fmt, rate)
            self.pipeline.set_state(Gst.State.PLAYING)
        pass

    def get_format_list(self):
        self.source.set_state (Gst.State.PAUSED)
        pad = self.source.pads[0]
        caps = pad.query_caps()

        l = []

        for i in range (caps.get_size()):
            s = caps.get_structure(i)
            text = "%s %dx%d" % (s.get_string("format"),
                                 s.get_int("width")[1],
                                 s.get_int("height")[1])
            try:
                s.get_value("framerate")
            except TypeError:
                # Workaround for missing GstValueList support in GI
                substr = s.to_string()[s.to_string().find("framerate="):]
                field,values,remain = re.split("{|}", substr, maxsplit=3)
                fpslist = [ ("%.0f" % (float(x.split("/")[0]) / float(x.split("/")[1])),
                             x.strip())
                            for x in values.split(",")]
            l.append((text, i, fpslist))

        self.source.set_state (Gst.State.NULL)

        return l

    def create_pipeline(self, fmt, rate):
        def bus_sync_handler(bus, msg, pipeline):
            if not GstVideo.is_video_overlay_prepare_window_handle_message(msg):
                return Gst.BusSyncReply.PASS
            msg.src.set_window_handle (self.da.get_window().get_xid())
            return Gst.BusSyncReply.DROP

        if self.source.get_parent() != None:
            raise (RuntimeError, "tcamsrc already has a parent")

        p = Gst.Pipeline()
        p.add(self.source)

        p.set_state (Gst.State.PAUSED)
        srccaps = self.source.pads[0].query_caps()
        structure = srccaps.get_structure(fmt).copy()
        structure.remove_field("framerate")

        flt = Gst.ElementFactory.make("capsfilter")
        capsstring = structure.to_string()[:-1]
        capsstring += ",framerate=(fraction)%s;" % (rate,)
        caps = Gst.Caps.from_string (capsstring)
        flt.set_property("caps", caps)
        print ( "Caps String: " + caps.to_string())

        bayerconvert = [("tcamwhitebalance","whitebalance"), ("bayer2rgb",),("videoconvert",)]
        converters = { "GRAY8": [("videoconvert",)],
                       "bggr": bayerconvert,
                       "grbg": bayerconvert,
                       "gbrg": bayerconvert,
                       "rggb": bayerconvert,
                       "GRAY16_LE" : [("videoconvert",)] }
        p.add (flt)
        self.source.link(flt)

        colorformat = structure.get_string("format")
        prev_elem = flt
        for conv in converters[colorformat]:
            elem = Gst.ElementFactory.make (*conv)
            p.add(elem)
            prev_elem.link(elem)
            prev_elem = elem

        queue1 = Gst.ElementFactory.make ("queue")
        p.add (queue1)
        prev_elem.link(queue1)

        sink = Gst.ElementFactory.make ("glimagesink")
        p.add (sink)
        queue1.link(sink)

        bus = p.get_bus()
        bus.set_sync_handler (bus_sync_handler, p)

        return p

if __name__ == "__main__":
    Gst.init ([])
    Gtk.init ([])

    source = None

    if len(sys.argv) == 2:
        serial = sys.argv[1]
        source = Gst.ElementFactory.make ("tcamsrc")
        source.set_property ("serial", serial)
    else:
        dlg = DeviceDialog()
        dlg.show_all()

        resp = dlg.run()

        if resp != Gtk.ResponseType.OK:
            sys.exit(0)

        source = dlg.get_source()
        dlg.destroy()

    #formats = get_format_list(serial)
    #print formats

    win = AppWindow(source)
    win.present()

    Gtk.main()
