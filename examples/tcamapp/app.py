import gi

gi.require_version ("Gtk", "3.0")
gi.require_version ("Gst", "1.0")
gi.require_version ("Tcam", "0.1")

from gi.repository import GdkX11, Gtk, Tcam, GstVideo, Gst, GdkPixbuf

import sys



class DeviceDialog (Gtk.Dialog):
    RESPONSE_OK = 1
    RESPONSE_CANCEL = 2
    def __init__(self, parent=None):
        Gtk.Dialog.__init__(self, parent)
        self.add_buttons (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
                          Gtk.STOCK_OK, Gtk.ResponseType.OK)

        model = Gtk.ListStore(GdkPixbuf.Pixbuf, str, str)
        self.__iv = Gtk.IconView.new_with_model(model)
        self.__iv.set_pixbuf_column(0)
        self.__iv.set_text_column(1)
        self.__iv.set_selection_mode (Gtk.SelectionMode.BROWSE)

        for dev in self.__get_devices():
            pixbuf = Gtk.IconTheme.get_default().load_icon (
                Gtk.STOCK_YES, 64, 0)
            label = "%s (%s)" % (dev[1], dev[0])
            model.append ((pixbuf, label, dev[0]))
        self.get_content_area().add(self.__iv)

    def __get_devices(self):
        elem = Gst.ElementFactory.make("tcamsrc")
        if not elem:
            raise (RuntimeError, "Failed to create tcamsrc element")
        ret = []
        for serial in elem.get_device_serials():
            result, name, ident, conn_type = elem.get_device_info(serial)
            ret.append( (serial, name, ident, conn_type))
        return ret

    def get_serial(self):
        return self.__iv.get_model()[self.__iv.get_selected_items()[0]][2]

class AppWindow (Gtk.Window):
    def __init__ (self, serial):
        Gtk.Window.__init__(self)

        self.serial = serial
        self.pipeline = None

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

    def create_format_combo (self):
        formats = self.get_format_list(self.serial)
        model = Gtk.ListStore (str, int)
        for fmt in formats:
            model.append (fmt)
        combo = Gtk.ComboBox.new_with_model (model)
        renderer_text = Gtk.CellRendererText()
        combo.pack_start (renderer_text, True)
        combo.add_attribute (renderer_text, "text", 0)
        #combo.set_active(0)
        return combo

    def on_format_combo_changed (self, combo):
        if self.pipeline:
            self.pipeline.set_state(Gst.State.NULL)
            self.pipeline.get_state(0)
        it = combo.get_active_iter()
        if it != None:
            model = combo.get_model()
            fmt = model[it][1]
            self.pipeline = self.create_pipeline(fmt)
            self.pipeline.set_state(Gst.State.PLAYING)

    def get_format_list(self, serial):
        elem = Gst.ElementFactory.make("tcamsrc")
        elem.set_property("serial", serial)

        elem.set_state(Gst.State.PAUSED)

        pad = elem.pads[0]
        caps = pad.query_caps()

        l = []

        for i in range (caps.get_size()):
            s = caps.get_structure(i)
            text = "%s %dx%d" % (s.get_string("format"),
                                 s.get_int("width")[1],
                                 s.get_int("height")[1])
            l.append((text, i))
        elem.set_state(Gst.State.NULL)

        return l

    def create_pipeline(self, fmt):
        def bus_sync_handler(bus, msg, pipeline):
            if not GstVideo.is_video_overlay_prepare_window_handle_message(msg):
                return Gst.BusSyncReply.PASS
            msg.src.set_window_handle (self.da.get_window().get_xid())
            return Gst.BusSyncReply.DROP

        p = Gst.Pipeline()
        src = Gst.ElementFactory.make("tcamsrc")
        src.set_property("serial", self.serial)
        p.add(src)

        p.set_state (Gst.State.PAUSED)
        srccaps = src.pads[0].query_caps()
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
        prev_elem = src
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

    serial = None

    if len(sys.argv) == 2:
        serial = sys.argv[1]
    else:
        dlg = DeviceDialog()
        dlg.show_all()

        resp = dlg.run()

        if resp != Gtk.ResponseType.OK:
            sys.exit(0)

            serial = dlg.get_serial()
        dlg.destroy()

    #formats = get_format_list(serial)
    #print formats

    win = AppWindow(serial)
    win.present()

    Gtk.main()
