import gi

gi.require_version ("Gtk", "3.0")
gi.require_version ("Gst", "1.0")
gi.require_version ("Tcam", "0.1")

from gi.repository import Gtk, Tcam, Gst, GdkPixbuf

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
            raise RuntimeError, "Failed to create tcamsrc element"
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

        self.set_title ("TCam Demo Applikation")
        self.connect ("destroy", Gtk.main_quit)

        hb = Gtk.HeaderBar()
        hb.set_show_close_button (True)
        hb.props.title = ("TCam Demo Applikation")
        self.set_titlebar (hb)
        hb.pack_start (self.create_format_combo())
        hb.show_all()

        vbox = Gtk.Box(Gtk.Orientation.VERTICAL)
        self.add(vbox)

        self.da = Gtk.DrawingArea()
        self.da.set_size_request (640, 480)
        vbox.pack_start (self.da, True, True, 0)

        vbox.show_all()

    def create_format_combo (self):
        formats = self.get_format_list(self.serial)
        model = Gtk.ListStore (str, Gst.Structure)
        for fmt in formats:
            model.append (fmt)
        combo = Gtk.ComboBox.new_with_model (model)
        renderer_text = Gtk.CellRendererText()
        combo.pack_start (renderer_text, True)
        combo.add_attribute (renderer_text, "text", 0)
        combo.set_active(0)
        return combo

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
            l.append((text, s))
        elem.set_state(Gst.State.NULL)

        return l


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
