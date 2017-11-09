#pragma once

#include <gtkmm.h>

#include "tcamcamera.h"
#include "property_box.h"

using namespace gsttcam;

class TcamView
{
public:
    TcamView();
    Gtk::ApplicationWindow *get_window();

private:
    void update_device_combo();
    void on_device_changed();
    void update_formats();
    void update_frame_rates();
    void on_frame_rate_changed();

    TcamCamera* cam_ = nullptr;

    std::unique_ptr<PropertyBox> property_box_ = nullptr;

    Glib::RefPtr<Gtk::Builder> builder_;

    class DeviceModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        DeviceModelColumns()
        {add(col_serial_); add(col_name_);}
        ~DeviceModelColumns() {};
        Gtk::TreeModelColumn<std::string> col_serial_;
        Gtk::TreeModelColumn<std::string> col_name_;
    };
    DeviceModelColumns device_columns_;

    class FormatModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        FormatModelColumns()
        {add(col_description_); add(col_tcam_format_caps_);}
        ~FormatModelColumns() {};
        Gtk::TreeModelColumn<std::string> col_description_;
        Gtk::TreeModelColumn<VideoFormatCaps> col_tcam_format_caps_;
    };
    FormatModelColumns format_columns_;

    class FramerateModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        FramerateModelColumns()
        {add(col_description_); add(col_size_); add(col_framerate_);}
        ~FramerateModelColumns() {};
        Gtk::TreeModelColumn<std::string> col_description_;
        Gtk::TreeModelColumn<FrameSize> col_size_;
        Gtk::TreeModelColumn<FrameRate> col_framerate_;
    };
    FramerateModelColumns framerate_columns_;
};
