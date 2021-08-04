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

#include "PropertyCategory.h"
#include "PropertyFlags.h"
#include "base_types.h"
#include "error.h"
#include "visibility.h"

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace tcam::property
{

class IPropertyBase
{
public:
    virtual ~IPropertyBase() = default;

    virtual std::string get_name() const = 0;
    virtual TCAM_PROPERTY_TYPE get_type() const = 0;
    virtual PropertyFlags get_flags() const = 0;
};


std::shared_ptr<tcam::property::IPropertyBase> find_property(
    const std::vector<std::shared_ptr<tcam::property::IPropertyBase>>& properties,
    const std::string_view& name);


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

    virtual outcome::result<int64_t> get_value() const = 0;
    virtual outcome::result<void> set_value(int64_t new_value) = 0;
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

    virtual outcome::result<double> get_value() const = 0;
    virtual outcome::result<void> set_value(double new_value) = 0;
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

    virtual outcome::result<bool> get_value() const = 0;
    virtual outcome::result<void> set_value(bool new_value) = 0;
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

    virtual outcome::result<void> execute() = 0;
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

    virtual outcome::result<void> set_value_str(const std::string& new_value) = 0;
    virtual outcome::result<void> set_value(int64_t new_value) = 0;

    virtual outcome::result<std::string> get_value() const = 0;
    virtual outcome::result<int64_t> get_value_int() const = 0;

    virtual std::string get_default() const = 0;

    virtual std::vector<std::string> get_entries() const = 0;
};


} // namespace tcam::property
