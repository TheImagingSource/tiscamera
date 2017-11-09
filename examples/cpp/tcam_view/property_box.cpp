
#include <map>
#include <iostream>

#include "property_box.h"
#include "logscale.h"


using namespace gsttcam;

using std::unique_ptr;

PropertyBox::PropertyBox()
{
}

PropertyBox::~PropertyBox()
{
}

struct PropertyTabInfo
{
    PropertyTabInfo(std::string _category) : category(_category) {}
    std::string category;
    std::vector<std::shared_ptr<Property>> properties;
};

struct comp
{
    bool operator()(const PropertyTabInfo& a, const PropertyTabInfo& b)
    {
        if(a.category == b.category)
            return a.category < b.category;
        else
            return a.category < b.category;
    }
};

Gtk::Box *
PropertyBox::create_property_widget(std::shared_ptr<Property> prop,
                                    Glib::RefPtr<Gtk::SizeGroup> szgroup,
                                    TcamCamera &cam)
{
    Gtk::Box* box = new Gtk::Box();
    box->set_orientation(Gtk::Orientation::ORIENTATION_HORIZONTAL);
    Gtk::Label* label = new Gtk::Label(prop->name);
    szgroup->add_widget(*label);
    box->pack_start(*label, FALSE, FALSE, 4);
    widgets_.push_back(static_cast<Gtk::Widget*>(label));
    if(prop->type == "integer")
    {
        std::shared_ptr<IntegerProperty> ip = std::dynamic_pointer_cast<IntegerProperty>(prop);
        int tmp;
        ip->get(cam, tmp);
        if (ip->max <= ip->min)
            return box;
        LogScale *sc = new LogScale(Gtk::Adjustment::create(
            ip->value, ip->min, ip->max, ip->step_size, ip->step_size,
            ip->step_size), Gtk::Orientation::ORIENTATION_HORIZONTAL);
        sc->set_digits(0);
        box->pack_start(*sc, TRUE, TRUE, 0);
        widgets_.push_back(static_cast<Gtk::Widget*>(sc));
        sc->signal_value_changed().connect(
            [this, sc, &cam, ip] () {
                auto value = sc->get_value();
                ip->set(cam, static_cast<int>(value));
            }
        );
    }
    else if(prop->type == "double")
    {
        std::shared_ptr<DoubleProperty> dp = std::dynamic_pointer_cast<DoubleProperty>(prop);
        if (dp->max <= dp->min)
            return box;
        LogScale *sc = new LogScale(Gtk::Adjustment::create(
            dp->value, dp->min, dp->max, dp->step_size, dp->step_size,
            dp->step_size), Gtk::Orientation::ORIENTATION_HORIZONTAL);
        box->pack_start(*sc, TRUE, TRUE, 0);
        widgets_.push_back(static_cast<Gtk::Widget*>(sc));
        sc->signal_value_changed().connect(
            [this, sc, &cam, dp] () {
                auto value = sc->get_value();
                dp->set(cam, static_cast<double>(value));
            }
        );
        std::cout << prop->to_string() << std::endl;
    }
    else if(prop->type == "enum")
    {
        std::shared_ptr<EnumProperty> ep = std::dynamic_pointer_cast<EnumProperty>(prop);
        Gtk::ComboBoxText *cbt = new Gtk::ComboBoxText();
        box->pack_start(*cbt, TRUE, TRUE, 0);
        widgets_.push_back(static_cast<Gtk::Widget*>(cbt));
        for(auto val : ep->values)
        {
            cbt->append(val);
        }
        cbt->set_active_text(ep->value);
        cbt->signal_changed().connect(
            [this, cbt, &cam, ep] () {
                std::cout << "Set: " << cbt->get_active_text() << std::endl;
                ep->set(cam, cbt->get_active_text());
            }
        );
    }
    else if(prop->type == "boolean")
    {
        std::shared_ptr<BooleanProperty> bp = std::dynamic_pointer_cast<BooleanProperty>(prop);
        Gtk::ToggleButton *tb = new Gtk::ToggleButton("Enable");
        box->pack_start(*tb, TRUE, TRUE, 0);
        widgets_.push_back(static_cast<Gtk::Widget*>(tb));
        tb->set_active(bp->value);
        tb->signal_toggled().connect(
            [this, tb, &cam, bp] () {
                bp->set(cam, tb->get_active());
            }
        );
    }
    else if(prop->type == "button")
    {
        std::shared_ptr<BooleanProperty> bp = std::dynamic_pointer_cast<BooleanProperty>(prop);
        Gtk::Button *bt = new Gtk::Button("Enable");
        box->pack_start(*bt, TRUE, TRUE, 0);
        widgets_.push_back(static_cast<Gtk::Widget*>(bt));
        bt->signal_clicked().connect(
            [this, bt, &cam, bp] () {
                bp->set(cam, true);
            }
        );
    }

    return box;
}

void
PropertyBox::populate_tabs(TcamCamera& cam)
{
    auto properties = cam.get_camera_property_list();
    std::map<std::string, std::vector<std::shared_ptr<Property>>> ppty_map;
    std::vector<std::string> categories;
    for(auto property : properties)
    {
        if(ppty_map.find(property->category) == ppty_map.end())
        {
            ppty_map[property->category];
            categories.push_back(property->category);
        }
        ppty_map[property->category].push_back(property);
    }
    for(auto category : categories)
    {
        Gtk::Box *box = new Gtk::Box();
        auto szgroup = Gtk::SizeGroup::create(Gtk::SizeGroupMode::SIZE_GROUP_HORIZONTAL);
        box->set_orientation(Gtk::Orientation::ORIENTATION_VERTICAL);
        for(auto property : ppty_map[category])
        {
            Gtk::Box* property_box = create_property_widget(property, szgroup, cam);
            box->pack_start(*property_box, false, false, 6);
            widgets_.push_back(property_box);
        }
        append_page(*box, category);
        widgets_.push_back(box);
    }
    show_all_children();
}