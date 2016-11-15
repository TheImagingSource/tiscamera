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

#include "Properties.h"
#include "logging.h"
#include "utils.h" // tcam_xioctl

#include <algorithm> // find_if
#include <cstring>

using namespace tcam;




PropertyString::PropertyString (std::shared_ptr<PropertyImpl> prop_impl,
                                const tcam_device_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertyString::~PropertyString ()
{}


std::string PropertyString::get_default () const
{
    return prop.value.s.default_value;
}


bool PropertyString::set_value (const std::string& new_value)
{
    if (is_read_only())
    {
        return false;
    }

    if (new_value.size() > sizeof(this->prop.value.s.value))
        return false;

    memcpy(this->prop.value.s.value, new_value.c_str(), sizeof(this->prop.value.s.value));

    notify_impl();

    return true;
}


std::string PropertyString::get_value () const
{
    return prop.value.s.value;
}






PropertyEnumeration::PropertyEnumeration (std::shared_ptr<PropertyImpl> prop_impl,
                                          const tcam_device_property& prop,
                                          const std::map<std::string, int>& values,
                                          VALUE_TYPE t)
    : Property(prop, values, t)
{
    impl = prop_impl;
}


PropertyEnumeration::~PropertyEnumeration ()
{}


std::vector<std::string> PropertyEnumeration::get_values () const
{
    std::vector<std::string> vec;

    for (auto m : string_map)
    {
        vec.push_back(std::get<0>(m));
    }

    return vec;
}


std::string PropertyEnumeration::get_default () const
{
    for (const auto& s : string_map)
    {
        if (prop.value.i.default_value == s.second)
        {
            return s.first;
        }
    }
    return "";
}


bool PropertyEnumeration::set_value (const std::string& new_value)
{
    if (is_read_only())
    {
        return false;
    }

    auto element = string_map.find(new_value);

    if (element == string_map.end())
    {
        return false;
    }

    prop.value.i.value = element->second;

    notify_impl();

    return false;
}


std::string PropertyEnumeration::get_value () const
{
    for (const auto& s : string_map)
    {
        if (prop.value.i.value == s.second)
        {
            return s.first;
        }
    }

    return "";
}


std::map<std::string, int> PropertyEnumeration::get_mapping () const
{
    return string_map;
}





PropertyBoolean::PropertyBoolean (std::shared_ptr<PropertyImpl> prop_impl,
                                const tcam_device_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertyBoolean::~PropertyBoolean ()
{}


bool PropertyBoolean::get_default () const
{
    return prop.value.b.default_value;
}


bool PropertyBoolean::set_value (bool value)
{
    if (is_read_only())
    {
        return false;
    }

    prop.value.b.value = value;
    notify_impl();

    return true;
}


bool PropertyBoolean::get_value () const
{
    return prop.value.b.value;
}






PropertyInteger::PropertyInteger (std::shared_ptr<PropertyImpl> prop_impl,
                                  const tcam_device_property& prop,
                                  VALUE_TYPE t)
    : Property (prop, t)
{
    impl = prop_impl;
}


PropertyInteger::~PropertyInteger ()
{}


int64_t PropertyInteger::get_default () const
{
    return prop.value.i.default_value;
}


int64_t PropertyInteger::get_min () const
{
    return this->prop.value.i.min;
}


int64_t PropertyInteger::get_max () const
{
    return this->prop.value.i.max;
}


int64_t PropertyInteger::get_step () const
{
  return this->prop.value.i.step;
}


int64_t PropertyInteger::get_value () const
{
    return this->prop.value.i.value;
}


bool PropertyInteger::set_value (int64_t new_value)
{
    // if (is_read_only())
    // return false;

    tcam_value_int& i = this->prop.value.i;

    if (i.min > new_value || i.max < new_value)
        return false;


    if (i.step > 0 && new_value % i.step != 0)
    {
        return false;
    }

    i.value = new_value;

    notify_impl();

    return true;
}






PropertyDouble::PropertyDouble (std::shared_ptr<PropertyImpl> prop_impl,
                                const tcam_device_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertyDouble::~PropertyDouble ()
{}


double PropertyDouble::get_default () const
{
    return prop.value.d.default_value;
}


double PropertyDouble::get_min () const
{
    return this->prop.value.d.min;
}


double PropertyDouble::get_max () const
{
    return this->prop.value.d.max;
}


double PropertyDouble::get_step () const
{
    return this->prop.value.d.step;
}


double PropertyDouble::get_value () const
{
    return this->prop.value.d.value;
}


bool PropertyDouble::set_value (double new_value)
{
    if (is_read_only())
    {
        return false;
    }

    tcam_value_double& d = this->prop.value.d;

    if (d.min > new_value || d.max < new_value)
    {
        return false;
    }

    d.value = new_value;

    notify_impl();

    return false;
}





PropertyButton::PropertyButton (std::shared_ptr<PropertyImpl> prop_impl,
                                const tcam_device_property& prop,
                                VALUE_TYPE t)
    : Property(prop, t)
{
    impl = prop_impl;
}


PropertyButton::~PropertyButton ()
{}


bool PropertyButton::activate ()
{
    if (is_read_only())
        return false;

    notify_impl();

    return true;
}
