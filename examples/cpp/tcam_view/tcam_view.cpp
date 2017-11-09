#include "tcam_view.h"
#include "property_box.h"

#include <iostream>
#include <set>
#include <memory>

TcamView::TcamView()
{
    builder_ = Gtk::Builder::create_from_file("tcam_view.glade");

    Gtk::ApplicationWindow *window = nullptr;
    builder_->get_widget("MainWindow", window);

    Gtk::ComboBox *format_combo = nullptr;
    builder_->get_widget("FormatCombo", format_combo);
    format_combo->pack_start(format_columns_.col_description_);

    Gtk::ComboBox *framerate_combo = nullptr;
    builder_->get_widget("FramerateCombo", framerate_combo);
    framerate_combo->pack_start(framerate_columns_.col_description_);

    Gtk::Paned *paned = nullptr;
    builder_->get_widget("Paned", paned);
    paned->set_position(300);

    update_device_combo();
    window->set_default_size(800, 600);
    window->show_all();
}

void
TcamView::update_device_combo()
{
    Glib::RefPtr<Gtk::ListStore> model;
    model = Gtk::ListStore::create(device_columns_);
    Gtk::ComboBox *device_combo = nullptr;
    builder_->get_widget("DeviceCombo", device_combo);
    for(auto device : get_device_list())
    {
        Gtk::TreeModel::Row row = *(model->append());
        row[device_columns_.col_name_] = device.name + "(" + device.serial + ")";
        row[device_columns_.col_serial_] = device.serial;
    }
    device_combo->pack_start(device_columns_.col_name_);
    device_combo->set_model(model);
    device_combo->signal_changed().connect(sigc::mem_fun(*this,
                                            &TcamView::on_device_changed));
}

void
TcamView::on_device_changed()
{
    if(cam_ != nullptr)
        delete cam_;
    Gtk::ComboBox *device_combo = nullptr;
    builder_->get_widget("DeviceCombo", device_combo);
    Gtk::TreeModel::iterator iter = device_combo->get_active();
    if (iter && *iter)
    {
        Gtk::TreeModel::Row row = *iter;
        cam_ = new TcamCamera(row[device_columns_.col_serial_]);
        update_formats();
        Gtk::Viewport *vp = nullptr;
        builder_->get_widget("ViewPort", vp);
        GstElement *displaysink = gst_element_factory_make("gtksink", NULL);
        GtkWidget *_widget;
        g_object_get(displaysink, "widget", &_widget, nullptr);
        Gtk::Widget *widget = Glib::wrap(_widget);
        if(vp->get_child())
            vp->remove();
        vp->add(*widget);
        cam_->enable_video_display(displaysink);
        vp->show_all();
    }
}

struct comp
{
    bool operator()(const VideoFormatCaps& a, const VideoFormatCaps& b)
    {
        if(a.size.width == b.size.width)
            return a.size.height < b.size.height;
        else
            return a.size.width < b.size.width;
    }
};

void
TcamView::update_formats()
{
    if(cam_ == nullptr)
        return;
    Glib::RefPtr<Gtk::ListStore> model;
    model = Gtk::ListStore::create(format_columns_);
    Gtk::ComboBox *format_combo = nullptr;
    builder_->get_widget("FormatCombo", format_combo);
    std::set<VideoFormatCaps, comp> sizes;
    for(auto format : cam_->get_format_list())
    {
        sizes.insert(format);
    }
    for(auto format : sizes)
    {
        Gtk::TreeModel::Row row = *(model->append());
        std::string description = std::to_string(format.size.width) + "x" +
                std::to_string(format.size.height);
        row[format_columns_.col_description_] = description;
        row[format_columns_.col_tcam_format_caps_] = format;
    }
    format_combo->set_model(model);
    format_combo->signal_changed().connect(sigc::mem_fun(*this,
                                            &TcamView::update_frame_rates));
}

void
TcamView::update_frame_rates()
{
    if(cam_ == nullptr)
        return;
    Gtk::ComboBox *format_combo = nullptr;
    builder_->get_widget("FormatCombo", format_combo);
    Gtk::TreeModel::iterator iter = *format_combo->get_active();
    VideoFormatCaps fmt;
    if(iter && *iter)
        fmt = (*iter)[format_columns_.col_tcam_format_caps_];
    else
        return;
    Glib::RefPtr<Gtk::ListStore> model;
    model = Gtk::ListStore::create(framerate_columns_);
    for(auto framerate : fmt.framerates)
    {
        Gtk::TreeModel::Row row = *(model->append());
        std::string description = std::to_string(framerate.numerator) + "/" +
                std::to_string(framerate.denominator);
        row[framerate_columns_.col_description_] = description;
        row[framerate_columns_.col_size_] = fmt.size;
        row[framerate_columns_.col_framerate_] = framerate;
    }
    Gtk::ComboBox *framerate_combo = nullptr;
    builder_->get_widget("FramerateCombo", framerate_combo);
    framerate_combo->set_model(model);
    framerate_combo->signal_changed().connect(sigc::mem_fun(*this,
                                            &TcamView::on_frame_rate_changed));
}

static bool
is_format_supported(std::string format, std::vector<VideoFormatCaps> caps)
{
    for(auto fmt : caps)
    {
        if(std::find(fmt.formats.begin(), fmt.formats.end(), format) != fmt.formats.end())
        {
            return true;
        }
    }
    return false;
}

void
TcamView::on_frame_rate_changed()
{
    Gtk::ComboBox *framerate_combo = nullptr;
    builder_->get_widget("FramerateCombo", framerate_combo);
    Gtk::TreeModel::iterator iter = *framerate_combo->get_active();
    FrameSize size;
    FrameRate rate;
    if(iter && *iter)
    {
        size = (*iter)[framerate_columns_.col_size_];
        rate = (*iter)[framerate_columns_.col_framerate_];
    }
    else
        return;
    std::string pixfmt = "BGRx";
    if (!is_format_supported(pixfmt, cam_->get_format_list()))
        pixfmt = "GRAY8";
    cam_->stop();
    cam_->set_capture_format(pixfmt, size, rate);
    cam_->start();

    Gtk::EventBox *ebox = nullptr;
    builder_->get_widget("PropertyBox", ebox);
    property_box_ = nullptr;
    property_box_ = std::unique_ptr<PropertyBox>(new PropertyBox());
    property_box_->populate_tabs(*cam_);
    ebox->add(*property_box_);
    ebox->show_all();
}

Gtk::ApplicationWindow*
TcamView::get_window()
{
    Gtk::ApplicationWindow *window = nullptr;
    builder_->get_widget("MainWindow", window);
    return window;
}
