


#include "Property.h"
#include "base_types.h"
#include "tis_logging.h"

#include <cstring>
#include <algorithm>

using namespace tis_imaging;


Property::Property ()
    : value_type(UNDEFINED), prop(), ref_prop()
{}


Property::Property (const camera_property& _property, const VALUE_TYPE& t)
    : prop(_property), ref_prop(_property), value_type(t)
{}


Property::Property (const camera_property& _property,
                    const std::map<std::string, int>& _map,
                    const VALUE_TYPE& t)
    : prop(_property), ref_prop(_property), string_map(_map), value_type(t)
{}


Property::~Property ()
{}


void Property::reset ()
{
    tis_log(TIS_LOG_INFO, "Resetting property to initial values.");
    prop = ref_prop;

    notifyImpl();
}


std::string Property::getName () const
{
    return prop.name;
}


PROPERTY_TYPE Property::getType () const
{
    return prop.type;
}


bool Property::isReadOnly () const
{
    return (prop.flags & (1 << PROPERTY_FLAG_READ_ONLY));
}


bool Property::isWriteOnly () const
{
    return (prop.flags & (1 << PROPERTY_FLAG_WRITE_ONLY));
}


bool Property::isDisabled () const
{
    return (prop.flags & (1 << PROPERTY_FLAG_DISABLED));
}


uint32_t Property::getFlags () const
{
    return prop.flags;
}


struct camera_property Property::getStruct () const
{
    return prop;
}


bool Property::setStruct (const struct camera_property& p)
{
    // TODO: only copy actual value;
    this->prop = p;
    return true;
}


Property::VALUE_TYPE Property::getValueType () const
{
    return value_type;
}


bool Property::setReadOnly (const bool only_read)
{
    if (only_read)
    {
        prop.flags |= 1 << PROPERTY_FLAG_READ_ONLY;
    }
    else
    {
        prop.flags &= ~(1 << PROPERTY_FLAG_READ_ONLY);
    }

    return true;
}


bool Property::setWriteOnly (const bool only_write)
{
    if (only_write)
    {
        prop.flags |= 1 << PROPERTY_FLAG_WRITE_ONLY;
    }
    else
    {
        prop.flags &= ~(1 << PROPERTY_FLAG_WRITE_ONLY);
    }

    return true;
}


bool Property::setInactive (const bool is_disabled)
{
    if (is_disabled)
    {
        prop.flags |= 1 << PROPERTY_FLAG_INACTIVE;
    }
    else
    {
        prop.flags &= ~(1 << PROPERTY_FLAG_INACTIVE);
    }

    return true;
}


bool Property::isAvailable (const Property&)
{
    // TODO:
    return false;
}


bool Property::setProperty (const Property&)
{
    if (impl.expired())
    {
        return false;
    }
    notifyImpl();

    return true;
}


bool Property::getProperty (Property&)
{
    // TODO:
    return true;
}


void Property::notifyImpl ()
{

    if (impl.expired())
    {
        tis_log(TIS_LOG_ERROR, "PropertyImpl expired. Property %s is corrupted.", this->getName().c_str());
    }

    auto ptr(impl.lock());

    ptr->setProperty(*this);
}
