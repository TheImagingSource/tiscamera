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

#include <memory>
#include <string>
#include <tcamprop1.0_base/tcamprop_property_info.h>

namespace tcam::property::emulated
{
template<class TBaseItf> class SoftwarePropertyImplBase : public TBaseItf
{
public:
    tcamprop1::prop_static_info get_static_info() const final
    {
        return *p_static_info;
    }

    PropertyFlags get_flags() const final
    {
        if (auto ptr = m_cam.lock())
        {
            return ptr->get_flags(m_id);
        }
        return tcam::property::PropertyFlags::None;
    }

protected:
    auto get_internal_name() const noexcept
    {
        return p_static_info->name;
    }

    SoftwarePropertyImplBase(software_prop id,
                             const tcamprop1::prop_static_info* info,
                             const std::shared_ptr<SoftwarePropertyBackend>& ptr)
        : m_id(id), m_cam(ptr), p_static_info(info)
    {
    }

    const software_prop m_id;
    std::weak_ptr<SoftwarePropertyBackend> m_cam;

    const tcamprop1::prop_static_info* p_static_info = nullptr;
};

class SoftwarePropertyIntegerImpl : public SoftwarePropertyImplBase<IPropertyInteger2>
{
public:
    SoftwarePropertyIntegerImpl(const std::shared_ptr<SoftwarePropertyBackend>& backend,
                                software_prop id,
                                const tcamprop1::prop_static_info_integer* info,
                                const prop_range_integer_def& range);

    tcamprop1::prop_static_info_integer get_static_info_ext() const override
    {
        return *static_info_integer_;
    }

    void set_range(const tcamprop1::prop_range_integer& new_range)
    {
        range_ = new_range;
    }

    tcamprop1::prop_range_integer get_range() const final
    {
        return range_;
    }

    outcome::result<int64_t> get_default() const final
    {
        if (m_default)
            return m_default.value();
        return tcam::status::PropertyNoDefaultAvailable;
    }

    outcome::result<int64_t> get_value() const final;
    outcome::result<void> set_value(int64_t new_value) final;

private:
    tcamprop1::prop_range_integer range_;

    std::optional<int64_t> m_default;

    const tcamprop1::prop_static_info_integer* static_info_integer_ = nullptr;
};


class SoftwarePropertyDoubleImpl : public SoftwarePropertyImplBase<IPropertyFloat2>
{
public:
    SoftwarePropertyDoubleImpl(const std::shared_ptr<SoftwarePropertyBackend>& backend,
                               software_prop id,
                               const tcamprop1::prop_static_info_float* info,
                               const prop_range_float_def& range);

    tcamprop1::prop_static_info_float get_static_info_ext() const override
    {
        return *static_info_float_;
    }
    tcamprop1::prop_range_float get_range() const final
    {
        return range_;
    }
    outcome::result<double> get_default() const final
    {
        if (m_default)
            return m_default.value();
        return tcam::status::PropertyNoDefaultAvailable;
    }
    outcome::result<double> get_value() const final;
    outcome::result<void> set_value(double new_value) final;

private:
    tcamprop1::prop_range_float range_;

    std::optional<double> m_default;

    const tcamprop1::prop_static_info_float* static_info_float_ = nullptr;
};


class SoftwarePropertyBoolImpl : public SoftwarePropertyImplBase<IPropertyBool>
{
public:
    SoftwarePropertyBoolImpl(const std::shared_ptr<SoftwarePropertyBackend>& backend,
                             software_prop id,
                             const tcamprop1::prop_static_info_boolean* info,
                             bool def);
    outcome::result<bool> get_default() const final
    {
        return m_default;
    }
    outcome::result<bool> get_value() const final;
    outcome::result<void> set_value(bool new_value) final;

private:
    bool m_default = false;

    //const tcamprop1::prop_static_info_boolean* p_static_info;
};


class SoftwarePropertyCommandImpl : public SoftwarePropertyImplBase<IPropertyCommand>
{
public:
    SoftwarePropertyCommandImpl(const std::shared_ptr<SoftwarePropertyBackend>& backend,
                                software_prop id,
                                const tcamprop1::prop_static_info_command* info);

    outcome::result<void> execute() final;
};


class SoftwarePropertyEnumImpl : public SoftwarePropertyImplBase<IPropertyEnum>
{
public:
    SoftwarePropertyEnumImpl(const std::shared_ptr<SoftwarePropertyBackend>& backend,
                             software_prop id,
                             const tcamprop1::prop_static_info_enumeration* info,
                             std::vector<std::string_view>&& entries,
                             int default_entry);

    outcome::result<void> set_value(std::string_view new_value) final;
    outcome::result<std::string_view> get_value() const final;

    outcome::result<std::string_view> get_default() const final
    {
        return m_default;
    }

    std::vector<std::string> get_entries() const final;

private:
    std::vector<std::string_view> m_entries;

    std::string_view m_default;
};


} // namespace tcam::property::emulated
