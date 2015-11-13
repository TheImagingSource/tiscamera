import gi

gi.require_version ("Gtk", "3.0")
gi.require_version ("Gst", "1.0")
gi.require_version ("Tcam", "0.1")
gi.require_version ("GdkX11", "3.0")
gi.require_version ("GstVideo", "1.0")

from gi.repository import GdkX11, Gtk, Tcam, GstVideo, Gst, GdkPixbuf

import sys



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

    def __create_main_vbox (self):
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

        return vbox

    def __create_control_vbox (self, name):
        vbox = Gtk.Box(orientation = Gtk.Orientation.VERTICAL)
        label = Gtk.Label (name)
        vbox.pack_start (label, True, False, 2)
        return vbox

    def __create_range_control (self,
                                name, minval, maxval, defval, curval, step):
        vbox = self.__create_control_vbox (name)
        scale = Gtk.HScale.new_with_range (minval, maxval, step)
        scale.set_value (curval)
        scale.add_mark (defval, Gtk.PositionType.TOP, None)
        vbox.pack_start (scale, True, True, 2)
        return vbox

    def __create_toggle_control (self, name, defval):
        vbox = self.__create_control_vbox (name)
        button = Gtk.ToggleButton.new_with_label (name)
        button.set_active (defval)
        vbox.pack_start (button, True, False, 2)
        return vbox

    def __create_button_control (self, name):
        button = self.__create_control_vbox (name)
        vbox.pack_start (button, True, False, 2)
        return vbox


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
        hb.show_all()

        vbox = Gtk.Box(Gtk.Orientation.VERTICAL)
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
        model = Gtk.ListStore (str, int)
        for fmt in formats:
            model.append (fmt)
        combo = Gtk.ComboBox.new_with_model (model)
        renderer_text = Gtk.CellRendererText()
        combo.pack_start (renderer_text, True)
        combo.add_attribute (renderer_text, "text", 0)
        return combo

    def on_format_combo_changed (self, combo):
        if self.pipeline:
            self.pipeline.set_state(Gst.State.NULL)
            self.pipeline.get_state(0)
            self.source.unparent()

        it = combo.get_active_iter()
        if it != None:
            model = combo.get_model()
            fmt = model[it][1]
            self.pipeline = self.create_pipeline(fmt)
            self.pipeline.set_state(Gst.State.PLAYING)

            #if self.ppty_dialog:
            #    self.ppty_dialog.destroy()
            #    self.ppty_dialog = PropertyDialog(
            #        self.pipeline.get_by_name ("source"))

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
            l.append((text, i))

        self.source.set_state (Gst.State.NULL)

        return l

    def create_pipeline(self, fmt):
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
        caps = Gst.Caps.from_string (structure.to_string())
        flt.set_property("caps", caps)
        print ( "Caps String: " + structure.to_string())

        converters = { "GRAY8": ("videoconvert",),
                       "gbrg": ("bayer2rgb","videoconvert"),
                       "GRAY16_LE" : ("videoconvert",) }

        colorformat = structure.get_string("format")
        prev_elem = self.source
        for conv in converters[colorformat]:
            elem = Gst.ElementFactory.make (conv)
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
    Gst.init()
    Gtk.init ()

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

    print (source, source.get_property("serial"))

    win = AppWindow(source)
    win.present()

    Gtk.main()
