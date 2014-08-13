


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


Property::Property (const Property& other)
{
    memcpy(&this->prop, &other.prop, sizeof(prop));
    this->impl = other.impl;
}


Property& Property::operator= (const Property& other)
{
    memcpy(&this->prop, &other.prop, sizeof(prop));
    return *this;
}


Property::~Property ()
{}


bool Property::operator== (const Property& other) const
{
    return (strcmp(this->prop.name, other.prop.name) == 0);
}


bool Property::operator== (const struct camera_property* other) const
{
    // return (memcmp(&this->prop, other, sizeof(prop)) == 0);
    return false;
}


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
    return (prop.flags & PROPERTY_FLAG_READ_ONLY);
}


bool Property::isWriteOnly () const
{
    return (prop.flags & PROPERTY_FLAG_WRITE_ONLY);
}


bool Property::isDisabled () const
{
    return (prop.flags & PROPERTY_FLAG_DISABLED);
}


uint32_t Property::getFlags () const
{
    return prop.flags;
}


struct camera_property Property::getStruct () const
{
    return prop;
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
