/*
 * Copyright 2014 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Property.h"
#include "base_types.h"
#include "logging.h"
#include "utils.h"

#include <cstring>
#include <algorithm>

using namespace tcam;


Property::Property ()
    : value_type(UNDEFINED), prop(), ref_prop()
{}


Property::Property (const tcam_device_property& property, VALUE_TYPE t)
    : value_type(t), prop(property), ref_prop(property)
{}


Property::Property (const tcam_device_property& property,
                    const std::map<std::string, int>& mapping,
                    VALUE_TYPE t)
    : value_type(t), prop(property), ref_prop(property), string_map(mapping)
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

    notify_impl();
}


bool Property::update ()
{
    if (auto ptr = impl.lock())
    {
        return ptr->get_property(*this);
    }
    return false;
}


TCAM_PROPERTY_ID Property::get_ID () const
{
    return prop.id;
}


std::string Property::get_name () const
{
    return prop.name;
}


TCAM_PROPERTY_TYPE Property::get_type () const
{
    return prop.type;
}


bool Property::can_be_changed () const
{
    return (is_read_only() && is_disabled());
}


bool Property::is_read_only () const
{
    return is_bit_set(prop.flags, TCAM_PROPERTY_FLAG_READ_ONLY);
}


bool Property::is_write_only () const
{
    return is_bit_set(prop.flags, TCAM_PROPERTY_FLAG_WRITE_ONLY);
}


bool Property::is_disabled () const
{
    return is_bit_set(prop.flags, TCAM_PROPERTY_FLAG_DISABLED);
}


bool Property::is_external () const
{
    return is_bit_set(prop.flags, TCAM_PROPERTY_FLAG_EXTERNAL);
}


uint32_t Property::get_flags () const
{
    return prop.flags;
}


struct tcam_device_property Property::get_struct () const
{
    return prop;
}


bool Property::set_struct (const struct tcam_device_property& p)
{
    set_struct_value(p);
    prop.flags = p.flags;
    return true;
}


void Property::set_struct_value (const struct tcam_device_property& p)
{
    switch (prop.type)
    {
        case TCAM_PROPERTY_TYPE_STRING:
            std::strncpy(prop.value.s.value, p.value.s.value, sizeof(prop.value.s.value));
        case TCAM_PROPERTY_TYPE_ENUMERATION:
            prop.value.i.value = p.value.i.value;
            // BUG string representation is not copied
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

void Property::get_struct_value (struct tcam_device_property& p)
{
    switch (prop.type)
    {
        case TCAM_PROPERTY_TYPE_STRING:
	    std::strncpy( p.value.s.value, prop.value.s.value, sizeof(prop.value.s.value));
	    std::strncpy( p.value.s.default_value, prop.value.s.default_value, sizeof(prop.value.s.default_value));
        case TCAM_PROPERTY_TYPE_ENUMERATION:
            p.value.i.value = prop.value.i.value;
	    p.value.i.min = prop.value.i.min;
	    p.value.i.max = prop.value.i.max;
	    p.value.i.default_value = prop.value.i.default_value;
	    p.value.i.step = prop.value.i.step;
            break;
        case TCAM_PROPERTY_TYPE_INTEGER:
            p.value.i.value = prop.value.i.value;
	    p.value.i.min = prop.value.i.min;
	    p.value.i.max = prop.value.i.max;
	    p.value.i.default_value = prop.value.i.default_value;
	    p.value.i.step = prop.value.i.step;
            break;
        case TCAM_PROPERTY_TYPE_DOUBLE:
            p.value.d.value = prop.value.d.value;
	    p.value.d.min = prop.value.d.min;
	    p.value.d.max = prop.value.d.max;
	    p.value.d.default_value = prop.value.d.default_value;
	    p.value.d.step = prop.value.d.step;
            break;
        case TCAM_PROPERTY_TYPE_BUTTON:
            // do nothing
            break;
        case TCAM_PROPERTY_TYPE_BOOLEAN:
            p.value.b.value = prop.value.b.value;
	    p.value.b.default_value = prop.value.b.default_value;
            break;
        case TCAM_PROPERTY_TYPE_UNKNOWN:
        default:
            break;
    }
};


Property::VALUE_TYPE Property::get_value_type () const
{
    return value_type;
}


std::string Property::to_string () const
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
        case TCAM_PROPERTY_TYPE_ENUMERATION:
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


bool Property::from_string (const std::string& s)
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
            case TCAM_PROPERTY_TYPE_ENUMERATION:
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


bool Property::set_property (const Property& p)
{
    if (impl.expired())
    {
        return false;
    }
    set_struct_value(p.get_struct());
    notify_impl();

    return true;
}


bool Property::set_value (const int64_t& value)
{
    if (impl.expired())
    {
        return false;
    }
    if (prop.value.i.min > value || prop.value.i.max < value)
    {
        return false;
    }
    if (get_type() == TCAM_PROPERTY_TYPE_INTEGER && !is_read_only())
    {
        prop.value.i.value = value;
        notify_impl();
        return true;
    }

    return false;
}


bool Property::set_value (const double& value)
{
    if (impl.expired())
    {
        return false;
    }
    if (prop.value.d.min > value || prop.value.d.max < value)
    {
        return false;
    }
    if (get_type() == TCAM_PROPERTY_TYPE_DOUBLE && !is_read_only())
    {
        prop.value.d.value = value;
        notify_impl();
        return true;
    }

    return false;
}


bool Property::set_value (const bool& value)
{
    if (impl.expired())
    {
        return false;
    }
    if (get_type() == TCAM_PROPERTY_TYPE_BOOLEAN && !is_read_only())
    {
        prop.value.b.value = value;
        notify_impl();
        return true;
    }

    return false;
}


bool Property::set_value (const std::string& value)
{
    if (impl.expired())
    {
        return false;
    }
    if (get_type() == TCAM_PROPERTY_TYPE_INTEGER && !is_read_only())
    {
        strcpy(prop.value.s.value, value.c_str());
        notify_impl();
        return true;
    }

    if (get_type() == TCAM_PROPERTY_TYPE_ENUMERATION && !is_read_only())
    {
        for (const auto& s : string_map)
        {
            if (value.compare(s.first) == 0)
            {
                prop.value.i.value = s.second;
                notify_impl();
                return true;
            }
        }
        return false;
    }

    return false;
}

bool Property::set_value ()
{
    if (impl.expired())
    {
        return false;
    }
    notify_impl();

    return true;
}


bool Property::set_property_from_struct (const tcam_device_property& prop)
{

    if (impl.expired())
    {
        return false;
    }
    set_struct_value(prop);
    notify_impl();

    return true;


}

bool Property::get_property_from_struct (tcam_device_property& prop)
{

    if (impl.expired())
    {
        return false;
    }
    get_struct_value(prop);

    return true;
}

bool Property::get_property (Property& p)
{
    p.set_struct(this->prop);
    return true;
}


void Property::notify_impl ()
{
    if (auto ptr = impl.lock())
    {
        ptr->set_property(*this);
    }
    else
    {
        tcam_log(TCAM_LOG_ERROR, "PropertyImpl expired. Property %s is corrupted.", this->get_name().c_str());
    }
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
            return TCAM_PROPERTY_TYPE_ENUMERATION;
        case Property::INTSWISSKNIFE:
        case Property::INTEGER:
            return TCAM_PROPERTY_TYPE_INTEGER;
        case Property::FLOAT:
            return TCAM_PROPERTY_TYPE_DOUBLE;
        case Property::BUTTON:
            return TCAM_PROPERTY_TYPE_BUTTON;
        case Property::COMMAND:
            return TCAM_PROPERTY_TYPE_BUTTON;
        default:
            return TCAM_PROPERTY_TYPE_UNKNOWN;
    };
}
