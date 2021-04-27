/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#pragma once

#include "base_types.h"

#include <string>
#include <vector>

namespace tcam::property
{

enum class Visibility
{
    Beginner = 0,
    Expert,
    Guru,
    Invisible,
};

class Category
{
    Visibility p_visibility = tcam::property::Visibility::Beginner;
    std::string p_name;
};


enum class PropertyFlags
{
    None = 0,
    Implemented = 1,
    Available = 2,
    Locked = 3,
    // additional flags like 'External' to indicate library and not camera internal properties?
};


inline PropertyFlags operator|(PropertyFlags a, PropertyFlags b)
{
    return static_cast<PropertyFlags>(static_cast<int>(a) | static_cast<int>(b));
}

inline PropertyFlags operator~(PropertyFlags a)
{
    return static_cast<PropertyFlags>(~static_cast<int>(a));
}

inline PropertyFlags operator&(PropertyFlags a, PropertyFlags b)
{
    return static_cast<PropertyFlags>(static_cast<int>(a) & static_cast<int>(b));
}

inline PropertyFlags operator^(PropertyFlags a, PropertyFlags b)
{
    return static_cast<PropertyFlags>(static_cast<int>(a) ^ static_cast<int>(b));
}

inline PropertyFlags& operator|=(PropertyFlags& a, PropertyFlags b)
{
    return (PropertyFlags&)((int&)a |= (int)b);
}

inline PropertyFlags& operator&=(PropertyFlags& a, PropertyFlags b)
{
    return (PropertyFlags&)((int&)a &= (int)b);
}

inline PropertyFlags& operator^=(PropertyFlags& a, PropertyFlags b)
{
    return (PropertyFlags&)((int&)a ^= (int)b);
}


class IPropertyBase
{
public:
    virtual ~IPropertyBase() = default;

    virtual std::string get_name() const = 0;
    virtual TCAM_PROPERTY_TYPE get_type() const = 0;
    virtual PropertyFlags get_flags() const = 0;
};

class IPropertyInteger : public IPropertyBase
{
public:
    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_INTEGER;
    }

    virtual std::string get_name() const override = 0;
    virtual PropertyFlags get_flags() const override = 0;

    virtual int64_t get_min() const = 0;
    virtual int64_t get_max() const = 0;
    virtual int64_t get_step() const = 0;
    virtual int64_t get_default() const = 0;
    virtual int64_t get_value() const = 0;

    virtual bool set_value(int64_t new_value) = 0;
};

class IPropertyFloat : public IPropertyBase
{
public:
    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_DOUBLE;
    }

    virtual std::string get_name() const override = 0;
    virtual PropertyFlags get_flags() const override = 0;

    virtual double get_min() const = 0;
    virtual double get_max() const = 0;
    virtual double get_step() const = 0;
    virtual double get_default() const = 0;
    virtual double get_value() const = 0;

    virtual bool set_value(double new_value) = 0;
};

class IPropertyBool : public IPropertyBase
{
public:
    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_BOOLEAN;
    }

    virtual std::string get_name() const override = 0;
    virtual PropertyFlags get_flags() const override = 0;

    virtual bool get_default() const = 0;
    virtual bool get_value() const = 0;

    virtual bool set_value(bool new_value) = 0;
};


class IPropertyCommand : public IPropertyBase
{
public:
    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_BUTTON;
    }

    virtual std::string get_name() const override = 0;
    virtual PropertyFlags get_flags() const override = 0;

    virtual bool execute() = 0;
};


class IPropertyEnum : public IPropertyBase
{
public:
    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_ENUMERATION;
    }

    virtual std::string get_name() const override = 0;
    virtual PropertyFlags get_flags() const override = 0;

    virtual bool set_value_str(const std::string& new_value) = 0;
    virtual bool set_value(int new_value) = 0;

    virtual std::string get_value() const = 0;
    virtual int get_value_int() const = 0;

    virtual std::string get_default() const = 0;

    virtual std::vector<std::string> get_entries() const = 0;
};


} // namespace tcam::property