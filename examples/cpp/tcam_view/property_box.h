#pragma once

#include <gtkmm.h>
#include <tcamcamera.h>

#include <vector>
#include <memory>

using namespace gsttcam;

class PropertyBox : public Gtk::Notebook
{
public:
    PropertyBox();
    virtual ~PropertyBox();
    void populate_tabs(TcamCamera &cam);

private:
    Gtk::Box* create_property_widget(std::shared_ptr<Property> prop,
                                    Glib::RefPtr<Gtk::SizeGroup> szgroup,
                                    TcamCamera &cam);
    std::vector<Gtk::Widget*> widgets_;
};