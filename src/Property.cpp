


#include "Property.h"
#include "base_types.h"
#include "logging.h"
#include "utils.h"
#include "Error.h"

#include <cstring>
#include <algorithm>

using namespace tcam;


Property::Property ()
    : value_type(UNDEFINED), prop(), ref_prop()
{}


Property::Property (const tcam_device_property& property, VALUE_TYPE t)
    : prop(property), ref_prop(property), value_type(t)
{}


Property::Property (const tcam_device_property& property,
                    const std::map<std::string, int>& mapping,
                    VALUE_TYPE t)
    : prop(property), ref_prop(property), string_map(mapping), value_type(t)
{}


Property::~Property ()
{}


Property& Property::operator= (const Property& other)
{
    this->prop = other.prop;
    // this->ref_prop = other.ref_prop;
    this->string_map = other.string_map;
    this->value_type = other.value_type;
    this->impl = other.impl;

    return *this;
}


void Property::reset ()
{
    tcam_log(TCAM_LOG_INFO, "Resetting property to initial values.");
    prop = ref_prop;

    notifyImpl();
}


bool Property::update ()
{
    if (impl.expired())
    {
        setError(Error("Property implementation has expired.", ENOENT));
        return false;
    }

    auto ptr = impl.lock();


    return ptr->getProperty(*this);
}


TCAM_PROPERTY_ID Property::getID () const
{
    return prop.id;
}


std::string Property::getName () const
{
    return prop.name;
}


TCAM_PROPERTY_TYPE Property::getType () const
{
    return prop.type;
}


bool Property::isReadOnly () const
{
    return is_bit_set(prop.flags, TCAM_PROPERTY_FLAG_READ_ONLY);
}


bool Property::isWriteOnly () const
{
    return is_bit_set(prop.flags, TCAM_PROPERTY_FLAG_WRITE_ONLY);
}


bool Property::isDisabled () const
{
    return is_bit_set(prop.flags, TCAM_PROPERTY_FLAG_DISABLED);
}


uint32_t Property::getFlags () const
{
    return prop.flags;
}


struct tcam_device_property Property::getStruct () const
{
    return prop;
}


bool Property::setStruct (const struct tcam_device_property& p)
{
    setStructValue(p);
    prop.flags = p.flags;
    return true;
}


void Property::setStructValue (const struct  tcam_device_property& p)
{
    switch (prop.type)
    {
        case TCAM_PROPERTY_TYPE_STRING:
            std::strncpy(prop.value.s.value, p.value.s.value, sizeof(prop.value.s.value));
        case TCAM_PROPERTY_TYPE_STRING_TABLE:
            prop.value.i.value = p.value.i.value;
            break;
        case TCAM_PROPERTY_TYPE_INTEGER:
            prop.value.i.value = p.value.i.value;
            break;
        case TCAM_PROPERTY_TYPE_DOUBLE:
            prop.value.d.value = p.value.d.value;
            break;
        case TCAM_PROPERTY_TYPE_BUTTON:
            // do nothing
            break;
        case TCAM_PROPERTY_TYPE_BOOLEAN:
            prop.value.b.value = p.value.b.value;
            break;
        case TCAM_PROPERTY_TYPE_UNKNOWN:
        default:
            break;
    }
};


Property::VALUE_TYPE Property::getValueType () const
{
    return value_type;
}


std::string Property::toString () const
{
    std::string property_string;

    switch (prop.type)
    {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            if (prop.value.b.value)
            {
                property_string += "true";
            }
            else
            {
                property_string += "false";
            }
            break;
        }
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            property_string += std::to_string(prop.value.i.value);
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            property_string += std::to_string(prop.value.d.value);
            break;
        }
        case TCAM_PROPERTY_TYPE_STRING:
        {
            property_string += prop.value.s.value;
            break;

        }
        case TCAM_PROPERTY_TYPE_STRING_TABLE:
        {

        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {

        }
        case TCAM_PROPERTY_TYPE_UNKNOWN:
        default:
        {

        }
    }

    return property_string;
}


bool Property::fromString (const std::string& s)
{
    try
    {
        switch (prop.type)
        {
            case TCAM_PROPERTY_TYPE_BOOLEAN:
            {
                if (s.compare("true") == 0)
                {
                    prop.value.b.value = true;
                }
                else
                {
                    prop.value.b.value = false;
                }
                break;
            }
            case TCAM_PROPERTY_TYPE_INTEGER:
            {
                prop.value.i.value = stoi(s);

                break;
            }
            case TCAM_PROPERTY_TYPE_DOUBLE:
            {
                prop.value.d.value = stod(s);
                break;
            }
            case TCAM_PROPERTY_TYPE_STRING:
            {
                strncpy(prop.value.s.value, s.c_str(), sizeof(prop.value.s.value));
                prop.value.s.value[sizeof(prop.value.s.value)-1] = '\0';
                break;
            }
            case TCAM_PROPERTY_TYPE_STRING_TABLE:
            {

            }
            case TCAM_PROPERTY_TYPE_BUTTON:
            case TCAM_PROPERTY_TYPE_UNKNOWN:
            default:
            {
                return false;
            }
        }
    }
    catch (const std::invalid_argument& e)
    {
        return false;
    }
    catch (const std::out_of_range& e)
    {
        return false;
    }

    return true;
}


bool Property::setProperty (const Property& p)
{
    if (impl.expired())
    {
        return false;
    }
    setStructValue(p.getStruct());
    notifyImpl();

    return true;
}


bool Property::getProperty (Property& p)
{
    p.setStruct(this->prop);
    return true;
}


void Property::notifyImpl ()
{
    if (impl.expired())
    {
        tcam_log(TCAM_LOG_ERROR, "PropertyImpl expired. Property %s is corrupted.", this->getName().c_str());
    }

    auto ptr(impl.lock());

    ptr->setProperty(*this);
}


TCAM_PROPERTY_TYPE tcam::value_type_to_ctrl_type (const Property::VALUE_TYPE& t)
{
    switch (t)
    {
        case Property::BOOLEAN:
            return TCAM_PROPERTY_TYPE_BOOLEAN;
        case Property::STRING:
            return TCAM_PROPERTY_TYPE_STRING;
        case Property::ENUM:
            return TCAM_PROPERTY_TYPE_STRING_TABLE;
        case Property::INTSWISSKNIFE:
        case Property::INTEGER:
            return TCAM_PROPERTY_TYPE_INTEGER;
        case Property::FLOAT:
            return TCAM_PROPERTY_TYPE_DOUBLE;
        case Property::BUTTON:
            return TCAM_PROPERTY_TYPE_BUTTON;
        case Property::COMMAND:
        default:
            return TCAM_PROPERTY_TYPE_UNKNOWN;
    };
}
