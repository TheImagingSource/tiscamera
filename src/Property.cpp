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
#include "Error.h"

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
    if (impl.expired())
    {
        setError(Error("Property implementation has expired.", ENOENT));
        return false;
    }

    auto ptr = impl.lock();


    return ptr->get_property(*this);
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


bool Property::get_property (Property& p)
{
    p.set_struct(this->prop);
    return true;
}


void Property::notify_impl ()
{
    if (impl.expired())
    {
        tcam_log(TCAM_LOG_ERROR, "PropertyImpl expired. Property %s is corrupted.", this->get_name().c_str());
    }

    auto ptr(impl.lock());

    ptr->set_property(*this);
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
