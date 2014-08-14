


#include "Property.h"
#include "tis_logging.h"

#include <cstring>
#include <algorithm>

using namespace tis_imaging;


Property::Property (const camera_property& _property)
{
    memcpy(&prop, &_property, sizeof(prop));
}

Property::Property (const camera_property& _property,
                    const std::map<int, std::string>& _map)
{
    memcpy(&prop, &_property, sizeof(prop));
}


Property::~Property ()
{}


void Property::reset ()
{
    // notifyListeners();
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
