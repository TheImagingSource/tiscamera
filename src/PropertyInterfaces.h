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

#include "PropertyFlags.h"
#include "base_types.h"
#include "error.h"

#include <memory>
#include <string>
#include <string_view>
#include <tcamprop1.0_base/tcamprop_base.h>
#include <tcamprop1.0_base/tcamprop_property_info.h>
#include <vector>

namespace tcam::property
{

class IPropertyBase
{
public:
    virtual ~IPropertyBase() = default;

    virtual tcamprop1::prop_static_info get_static_info() const = 0;

    std::string_view get_name() const noexcept
    {
        return get_static_info().name;
    }

    virtual TCAM_PROPERTY_TYPE get_type() const = 0;
    virtual PropertyFlags get_flags() const = 0;
};


std::shared_ptr<tcam::property::IPropertyBase> find_property(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& properties,
    std::string_view name);


class IPropertyInteger : public IPropertyBase
{
public:
    static const constexpr auto property_type = TCAM_PROPERTY_TYPE_INTEGER;

    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_INTEGER;
    }

    virtual std::string_view get_unit() const = 0;
    virtual tcamprop1::IntRepresentation_t get_representation() const = 0;

    virtual tcamprop1::prop_range_integer get_range() const = 0;
    virtual int64_t get_default() const = 0;

    virtual outcome::result<int64_t> get_value() const = 0;
    virtual outcome::result<void> set_value(int64_t new_value) = 0;
};

class IPropertyFloat : public IPropertyBase
{
public:
    static const constexpr auto property_type = tcam::TCAM_PROPERTY_TYPE_DOUBLE;

    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_DOUBLE;
    }

    virtual std::string_view get_unit() const = 0;
    virtual tcamprop1::FloatRepresentation_t get_representation() const = 0;

    virtual tcamprop1::prop_range_float get_range() const = 0;

    virtual double get_default() const = 0;

    virtual outcome::result<double> get_value() const = 0;
    virtual outcome::result<void> set_value(double new_value) = 0;
};

class IPropertyBool : public IPropertyBase
{
public:
    static const constexpr auto property_type = TCAM_PROPERTY_TYPE_BOOLEAN;

    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_BOOLEAN;
    }

    virtual bool get_default() const = 0;

    virtual outcome::result<bool> get_value() const = 0;
    virtual outcome::result<void> set_value(bool new_value) = 0;
};

class IPropertyCommand : public IPropertyBase
{
public:
    static const constexpr auto property_type = TCAM_PROPERTY_TYPE_BUTTON;

    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_BUTTON;
    }

    virtual outcome::result<void> execute() = 0;
};


class IPropertyEnum : public IPropertyBase
{
public:
    static const constexpr auto property_type = TCAM_PROPERTY_TYPE_ENUMERATION;

    TCAM_PROPERTY_TYPE get_type() const final
    {
        return TCAM_PROPERTY_TYPE_ENUMERATION;
    }

    virtual outcome::result<void> set_value_str(const std::string_view& new_value) = 0;

    virtual outcome::result<std::string_view> get_value() const = 0;

    virtual std::string get_default() const = 0;

    virtual std::vector<std::string> get_entries() const = 0;
};

class IPropertyString : public IPropertyBase
{
public:
    static const constexpr auto property_type = tcam::TCAM_PROPERTY_TYPE_STRING;

    TCAM_PROPERTY_TYPE get_type() const final
    {
        return tcam::TCAM_PROPERTY_TYPE_STRING;
    }

    virtual std::error_code set_value(std::string_view new_value) = 0;
    virtual outcome::result<std::string> get_value() const = 0;
};


template<class Tprop_type>
inline std::shared_ptr<Tprop_type> find_property(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& properties,
    std::string_view name)
{
    auto ptr = find_property(properties, name);
    if (ptr == nullptr)
    {
        return nullptr;
    }
    if (ptr->get_type() != Tprop_type::property_type)
    {
        return nullptr;
    }
    return std::static_pointer_cast<Tprop_type>(ptr);
}


class IPropertyInteger2 : public IPropertyInteger
{
public:
    virtual tcamprop1::prop_static_info_integer get_static_info_ext() const = 0;

    tcamprop1::prop_static_info get_static_info() const override
    {
        return get_static_info_ext();
    }
    std::string_view get_unit() const override
    {
        return get_static_info_ext().unit;
    }
    tcamprop1::IntRepresentation_t get_representation() const override
    {
        return get_static_info_ext().representation;
    }
};

class IPropertyFloat2 : public IPropertyFloat
{
public:
    virtual tcamprop1::prop_static_info_float get_static_info_ext() const = 0;

    tcamprop1::prop_static_info get_static_info() const override
    {
        return get_static_info_ext();
    }
    std::string_view get_unit() const override
    {
        return get_static_info_ext().unit;
    }
    tcamprop1::FloatRepresentation_t get_representation() const override
    {
        return get_static_info_ext().representation;
    }
};


} // namespace tcam::property
