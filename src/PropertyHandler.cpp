

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


bool PropertyHandler::setProperty (const Property& p)
{
    for (auto& prop : properties)
    {
        if (prop.external_property->getID() == p.getID())
        {
            if (prop.external_property->isReadOnly())
            {
                setError(Error("Property is read only.", EACCES));
                return false;
            }

            prop.internal_property->setProperty(p);
            prop.external_property->setStruct(p.getStruct());
        }
    }

    if (p.getID() == TCAM_PROPERTY_EXPOSURE_AUTO)
    {
        auto pea = find_property(emulated_properties, TCAM_PROPERTY_EXPOSURE_AUTO);
        bool vla = static_cast<PropertyBoolean&>(*pea).getValue();
        if (vla)
        {
            auto pe = find_property(external_properties, TCAM_PROPERTY_EXPOSURE);
            auto s = pe->getStruct();
            s.flags = set_bit(s.flags, TCAM_PROPERTY_FLAG_READ_ONLY);
            pe->setStruct(s);
        }
        else
        {
            auto pe = find_property(external_properties, TCAM_PROPERTY_EXPOSURE);
            auto s = pe->getStruct();
            s.flags = unset_bit(s.flags, TCAM_PROPERTY_FLAG_READ_ONLY);
            pe->setStruct(s);
        }
    }
    return false;
}


bool PropertyHandler::getProperty (Property& p)
{
    auto prop = find_mapping_external(p.getID());

    if (prop.internal_property != nullptr)
    {
        auto i = prop.internal_property;

        auto ext_struct = prop.external_property->getStruct();

        ext_struct.value = i->getStruct().value;

        prop.external_property->setStruct(ext_struct);
        p.setStruct(ext_struct);
        return true;
    }

    return false;
}


static bool is_wanted_property (const std::string& prop_name,  const std::shared_ptr<Property>& p)
{
    if (prop_name.compare(p->getName()) == 0)
        return true;
    return false;
}



PropertyHandler::property_mapping PropertyHandler::find_mapping_external (TCAM_PROPERTY_ID id)
{
    for (auto& m : properties)
    {
        if (m.external_property->getID() == id)
            return m;
    }
    return {nullptr, nullptr};
}


PropertyHandler::property_mapping PropertyHandler::find_mapping_internal (TCAM_PROPERTY_ID id)
{
    for (auto& m : properties)
    {
        if (m.internal_property->getID() == id)
            return m;
    }
    return {nullptr, nullptr};
}


std::shared_ptr<Property> create_property(const std::shared_ptr<Property>& p,
                                          std::shared_ptr<PropertyImpl> impl)
{
    TCAM_PROPERTY_TYPE type = p->getType();

    if (type == TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        return std::make_shared<Property>(PropertyBoolean(impl, p->getStruct(), p->getValueType()));
    }
    else if (type == TCAM_PROPERTY_TYPE_INTEGER)
    {
        return std::make_shared<Property>(PropertyInteger(impl, p->getStruct(), p->getValueType()));
    }
    else if (type == TCAM_PROPERTY_TYPE_DOUBLE)
    {
        return std::make_shared<Property>(PropertyDouble(impl, p->getStruct(), p->getValueType()));
    }
    else if (type == TCAM_PROPERTY_TYPE_STRING)
    {
        return std::make_shared<Property>(PropertyString(impl, p->getStruct(), p->getValueType()));
    }
    else if (type == TCAM_PROPERTY_TYPE_STRING_TABLE)
    {
        auto s = static_cast<PropertyStringMap&>(*p);

        return std::make_shared<Property>(PropertyStringMap(impl,
                                                            p->getStruct(),
                                                            s.getMapping(),
                                                            p->getValueType()));
    }
    else if (type == TCAM_PROPERTY_TYPE_BUTTON)
    {
        return std::make_shared<Property>(PropertyButton(impl, p->getStruct(), p->getValueType()));
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
}
