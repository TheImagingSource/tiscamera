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

#include "PropertyInterfaces.h"
#include "SoftwarePropertiesBase.h"

#include <map>
#include <memory>
#include <string>

using namespace tcam::property;

namespace tcam::property::emulated
{


class SoftwarePropertyBackend;

class SoftwarePropertyIntegerImpl : public IPropertyInteger
{
public:
    SoftwarePropertyIntegerImpl(
        const struct software_prop_desc* desc,
        std::shared_ptr<IPropertyInteger> prop,
        std::shared_ptr<SoftwarePropertyBackend> backend);

    SoftwarePropertyIntegerImpl(const struct software_prop_desc* desc,
                                std::shared_ptr<SoftwarePropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };
    virtual int64_t get_min() const final
    {
        return m_min;
    };
    virtual int64_t get_max() const final
    {
        return m_max;
    };
    virtual int64_t get_step() const final
    {
        return m_step;
    };
    virtual int64_t get_default() const final
    {
        return m_default;
    };

    virtual outcome::result<int64_t> get_value() const final;

    virtual outcome::result<void> set_value(int64_t new_value) final;

private:
    outcome::result<void> valid_value(int64_t val);

    std::weak_ptr<SoftwarePropertyBackend> m_cam;

    std::string m_name;
    PropertyFlags m_flags;

    int64_t m_min;
    int64_t m_max;
    int64_t m_step;
    int64_t m_default;

    software_prop m_id;
};


class SoftwarePropertyDoubleImpl : public IPropertyFloat
{
public:
    SoftwarePropertyDoubleImpl(const struct software_prop_desc* desc,
                               std::shared_ptr<IPropertyFloat> prop,
                               std::shared_ptr<SoftwarePropertyBackend> backend);
    SoftwarePropertyDoubleImpl(const struct software_prop_desc* desc,
                               std::shared_ptr<SoftwarePropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };
    virtual double get_min() const final
    {
        return m_min;
    };
    virtual double get_max() const final
    {
        return m_max;
    };
    virtual double get_step() const final
    {
        return m_step;
    };
    virtual double get_default() const final
    {
        return m_default;
    };
    virtual outcome::result<double> get_value() const final;

    virtual outcome::result<void> set_value(double new_value) final;

private:
    outcome::result<void> valid_value(double val);


    std::string m_name;
    PropertyFlags m_flags;

    double m_min;
    double m_max;
    double m_step;
    double m_default;

    software_prop m_id;
    std::weak_ptr<SoftwarePropertyBackend> m_cam;
};


class SoftwarePropertyBoolImpl : public IPropertyBool
{
public:
    SoftwarePropertyBoolImpl(const struct software_prop_desc* desc,
                             std::shared_ptr<SoftwarePropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };
    virtual bool get_default() const final
    {
        return m_default;
    };
    virtual outcome::result<bool> get_value() const final;

    virtual outcome::result<void> set_value(bool new_value) final;

private:
    std::weak_ptr<SoftwarePropertyBackend> m_cam;

    std::string m_name;
    PropertyFlags m_flags;

    bool m_default;

    software_prop m_id;
};


class SoftwarePropertyCommandImpl : public IPropertyCommand
{
public:
    SoftwarePropertyCommandImpl(const struct software_prop_desc* desc,
                                std::shared_ptr<SoftwarePropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };
    virtual outcome::result<void> execute() final;

private:
    std::weak_ptr<SoftwarePropertyBackend> m_cam;

    std::string m_name;
    PropertyFlags m_flags;

    software_prop m_id;
};


class SoftwarePropertyEnumImpl : public IPropertyEnum
{
public:
    SoftwarePropertyEnumImpl(const struct software_prop_desc* desc,
                             std::shared_ptr<SoftwarePropertyBackend> backend);

    virtual std::string get_name() const final
    {
        return m_name;
    };
    virtual PropertyFlags get_flags() const final
    {
        return m_flags;
    };
    virtual void set_flags(PropertyFlags flags) final
    {
        m_flags = flags;
    };

    virtual outcome::result<void> set_value_str(const std::string& new_value) final;
    virtual outcome::result<void> set_value(int64_t new_value) final;

    virtual outcome::result<std::string> get_value() const final;
    virtual outcome::result<int64_t> get_value_int() const final;

    virtual std::string get_default() const final
    {
        return m_default;
    };

    virtual std::vector<std::string> get_entries() const final;

private:
    bool valid_value(int value);

    std::map<int, std::string> m_entries;
    std::weak_ptr<SoftwarePropertyBackend> m_cam;

    std::string m_name;
    PropertyFlags m_flags;

    std::string m_default;

    software_prop m_id;
};


} // namespace tcam::property::emulated
