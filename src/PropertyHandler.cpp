

#include "PropertyHandler.h"

#include "Error.h"
#include "standard_properties.h"
#include "utils.h"
#include "logging.h"

#include <algorithm>

using namespace tcam;


PropertyHandler::PropertyHandler ()
{}


PropertyHandler::~PropertyHandler ()
{}


static bool is_group_master (const Property& p)
{
    if (p.get_struct().id == p.get_struct().group.property_group)
    {
        return true;
    }
    return false;
}


void PropertyHandler::group_properties ()
{
    groups.clear();

    for (auto& p : device_properties)
    {


        auto find_grouping = [&p] (const grouping& g)
            {
                if (g.id == p->get_struct().id)
                {
                    return true;
                }
                return false;
            };

        auto my_group = std::find_if(groups.begin(), groups.end(), find_grouping);

        if (my_group == groups.end())
        {
            grouping new_g = {};
            new_g.id = p->get_struct().id;
            if (is_group_master(*p))
            {
                new_g.master = p;
            }
            else
            {
                new_g.member.push_back(p);
            }
            groups.push_back(new_g);
        }
        else
        {
            if (is_group_master(*p))
            {
                my_group->master = p;
            }
            else
            {
                my_group->member.push_back(p);
            }
        }
    }
}

bool PropertyHandler::set_properties (std::vector<std::shared_ptr<Property>> device,
                                      std::vector<std::shared_ptr<Property>> emulated)
{
    clear();

    device_properties.insert(device_properties.end(), device.begin(), device.end());
    emulated_properties.insert(emulated_properties.end(), emulated.begin(), emulated.end());

    generate_properties();

    return true;
}


std::vector<std::shared_ptr<Property>> PropertyHandler::get_properties ()
{
    return external_properties;
}


void PropertyHandler::sync ()
{}


void PropertyHandler::clear ()
{
    properties.clear();
    external_properties.clear();

    device_properties.clear();
    emulated_properties.clear();
}


bool PropertyHandler::set_property (const Property& p)
{

    // Do the following things:
    // update the (internal)backend so that user values are used
    // update the (internal) representation of the exposed properties
    // check if other properties need to be changed (flags, etc).

    for (auto& prop : properties)
    {
        if (prop.external_property->get_ID() == p.get_ID())
        {
            if (prop.external_property->is_read_only())
            {
                setError(Error("Property is read only.", EACCES));
                tcam_log(TCAM_LOG_ERROR,
                         "Property '%s' is read only",
                         prop.external_property->get_name().c_str());
                return false;
            }

            prop.internal_property->set_property(p);
            prop.external_property->set_struct_value(prop.internal_property->get_struct());
            handle_flags(prop.external_property);
        }
    }

    return false;
}


bool PropertyHandler::get_property (Property& p)
{
    auto prop = find_mapping_external(p.get_ID());

    if (prop.internal_property != nullptr)
    {
        auto i = prop.internal_property;

        auto ext_struct = prop.external_property->get_struct();

        ext_struct.value = i->get_struct().value;

        prop.external_property->set_struct(ext_struct);
        p.set_struct(ext_struct);
        return true;
    }

    return false;
}


static bool is_wanted_property (const std::string& prop_name,  const std::shared_ptr<Property>& p)
{
    if (prop_name.compare(p->get_name()) == 0)
        return true;
    return false;
}



PropertyHandler::property_mapping PropertyHandler::find_mapping_external (TCAM_PROPERTY_ID id)
{
    for (auto& m : properties)
    {
        if (m.external_property->get_ID() == id)
            return m;
    }
    return {nullptr, nullptr};
}


PropertyHandler::property_mapping PropertyHandler::find_mapping_internal (TCAM_PROPERTY_ID id)
{
    for (auto& m : properties)
    {
        if (m.internal_property->get_ID() == id)
            return m;
    }
    return {nullptr, nullptr};
}


static std::shared_ptr<Property> create_property(const std::shared_ptr<Property>& p,
                                          std::shared_ptr<PropertyImpl> impl)
{
    TCAM_PROPERTY_TYPE type = p->get_type();

    if (type == TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        return std::make_shared<Property>(PropertyBoolean(impl, p->get_struct(), p->get_value_type()));
    }
    else if (type == TCAM_PROPERTY_TYPE_INTEGER)
    {
        return std::make_shared<Property>(PropertyInteger(impl, p->get_struct(), p->get_value_type()));
    }
    else if (type == TCAM_PROPERTY_TYPE_DOUBLE)
    {
        return std::make_shared<Property>(PropertyDouble(impl, p->get_struct(), p->get_value_type()));
    }
    else if (type == TCAM_PROPERTY_TYPE_STRING)
    {
        return std::make_shared<Property>(PropertyString(impl, p->get_struct(), p->get_value_type()));
    }
    else if (type == TCAM_PROPERTY_TYPE_STRING_TABLE)
    {
        auto s = static_cast<PropertyStringMap&>(*p);

        return std::make_shared<Property>(PropertyStringMap(impl,
                                                            p->get_struct(),
                                                            s.get_mapping(),
                                                            p->get_value_type()));
    }
    else if (type == TCAM_PROPERTY_TYPE_BUTTON)
    {
        return std::make_shared<Property>(PropertyButton(impl, p->get_struct(), p->get_value_type()));
    }
    return nullptr;
}


void PropertyHandler::generate_properties ()
{
    if (device_properties.empty() || emulated_properties.empty())
    {
        setError(Error("No properties to work with.", ENOENT));
        return;
    }

    auto self = shared_from_this();

    for (auto& p : device_properties)
    {
        auto new_p = create_property(p, self);

        if (new_p != nullptr)
        {
            external_properties.push_back(new_p);
            properties.push_back({new_p, p});
        }
    }
    for (auto& p : emulated_properties)
    {
        auto new_p = create_property(p, self);

        if (new_p != nullptr)
        {
            external_properties.push_back(new_p);
            properties.push_back({new_p, p});
        }
    }

    for (auto& p : external_properties)
    {
        handle_flags(p);
    }

}


void PropertyHandler::toggle_read_only (TCAM_PROPERTY_ID id, bool read_only)
{
    auto pe = find_property(external_properties, id);

    if (pe == nullptr)
        return;

    if (read_only)
        set_property_flag(pe, TCAM_PROPERTY_FLAG_READ_ONLY);
    else
        unset_property_flag(pe, TCAM_PROPERTY_FLAG_READ_ONLY);
}


void PropertyHandler::handle_flags (std::shared_ptr<Property>& p)
{

    switch (p->get_ID())
    {
        case TCAM_PROPERTY_EXPOSURE:
        {
            auto prop = find_mapping_internal(TCAM_PROPERTY_EXPOSURE_AUTO);

            if (prop.internal_property != nullptr)
            {
                if (((PropertyBoolean&) (*prop.internal_property)).get_value())
                {
                    set_property_flag(p, TCAM_PROPERTY_FLAG_READ_ONLY);
                }
                else
                {
                    unset_property_flag(p, TCAM_PROPERTY_FLAG_READ_ONLY);
                }
            }
            break;
        }
        case TCAM_PROPERTY_EXPOSURE_AUTO:
        {
            auto pea = find_property(emulated_properties, TCAM_PROPERTY_EXPOSURE_AUTO);

            bool vla = static_cast<PropertyBoolean&>(*pea).get_value();
            toggle_read_only(TCAM_PROPERTY_EXPOSURE, vla);

            break;
        }
        case TCAM_PROPERTY_GAIN:
        {
            auto prop = find_mapping_internal(TCAM_PROPERTY_GAIN_AUTO);

            if (prop.internal_property != nullptr)
            {
                if (((PropertyBoolean&) (*prop.internal_property)).get_value())
                {
                    set_property_flag(p, TCAM_PROPERTY_FLAG_READ_ONLY);
                }
                else
                {
                    unset_property_flag(p, TCAM_PROPERTY_FLAG_READ_ONLY);
                }
            }

            break;
        }
        case TCAM_PROPERTY_GAIN_AUTO:
        {
            auto pea = find_property(emulated_properties, TCAM_PROPERTY_GAIN_AUTO);

            bool vla = static_cast<PropertyBoolean&>(*pea).get_value();

            toggle_read_only(TCAM_PROPERTY_GAIN, vla);
            break;
        }
        case TCAM_PROPERTY_WB_AUTO:
        {
            // auto pea = find_property(emulated_properties, TCAM_PROPERTY_WB_RED);
            bool vla = static_cast<PropertyBoolean&>(*p).get_value();

            toggle_read_only(TCAM_PROPERTY_WB_RED,   vla);
            toggle_read_only(TCAM_PROPERTY_WB_GREEN, vla);
            toggle_read_only(TCAM_PROPERTY_WB_BLUE,  vla);

            break;
        }
        default:
            break;
    }
}


void PropertyHandler::set_property_flag (std::shared_ptr<Property>& p, TCAM_PROPERTY_FLAGS flag)
{
    auto s = p->get_struct();
    s.flags = set_bit(s.flags, flag);
    p->set_struct(s);
}


void PropertyHandler::unset_property_flag (std::shared_ptr<Property>& p, TCAM_PROPERTY_FLAGS flag)
{
    auto s = p->get_struct();
    s.flags = unset_bit(s.flags, flag);
    p->set_struct(s);
}
