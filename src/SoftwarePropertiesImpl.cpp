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

#include "SoftwarePropertiesImpl.h"

#include "logging.h"

using namespace tcam::property::emulated;

SoftwarePropertyIntegerImpl::SoftwarePropertyIntegerImpl(
    const std::shared_ptr<SoftwarePropertyBackend>& backend,
    software_prop id,
    const tcamprop1::prop_static_info_integer* info,
    const prop_range_integer_def& range)
    : SoftwarePropertyImplBase(id, info, backend)
{
    range_ = range.range;
    m_default = range.def;

    static_info_integer_ = info;
}

outcome::result<int64_t> SoftwarePropertyIntegerImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_int(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot read value.",
                     get_internal_name());
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> SoftwarePropertyIntegerImpl::set_value(int64_t new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_int(m_id, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.",
                     get_internal_name());
        return tcam::status::ResourceNotLockable;
    }
}

SoftwarePropertyDoubleImpl::SoftwarePropertyDoubleImpl(
    const std::shared_ptr<SoftwarePropertyBackend>& backend,
    software_prop id,
    const tcamprop1::prop_static_info_float* info,
    const prop_range_float_def& range)
    : SoftwarePropertyImplBase(id, info, backend), range_(range.range), m_default(range.def)
{
    static_info_float_ = info;
}

outcome::result<double> SoftwarePropertyDoubleImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_double(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot read value.",
                     get_internal_name());
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> SoftwarePropertyDoubleImpl::set_value(double new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_double(m_id, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.",
                     get_internal_name());
        return tcam::status::ResourceNotLockable;
    }
}

SoftwarePropertyBoolImpl::SoftwarePropertyBoolImpl(
    const std::shared_ptr<SoftwarePropertyBackend>& backend,
    software_prop id,
    const tcamprop1::prop_static_info_boolean* info,
    bool def)
    : SoftwarePropertyImplBase(id, info, backend), m_default(def)
{
}

outcome::result<bool> SoftwarePropertyBoolImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_id);

        if (ret)
        {
            return ret.value();
        }
        return ret.as_failure();
    }

    SPDLOG_ERROR("Unable to lock property backend for {}. Cannot read value.", get_internal_name());
    return tcam::status::ResourceNotLockable;
}

outcome::result<void> SoftwarePropertyBoolImpl::set_value(bool new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_int(m_id, new_value);
    }

    SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.",
                 get_internal_name());
    return tcam::status::ResourceNotLockable;
}

SoftwarePropertyCommandImpl::SoftwarePropertyCommandImpl(
    const std::shared_ptr<SoftwarePropertyBackend>& backend,
    software_prop id,
    const tcamprop1::prop_static_info_command* info)
    : SoftwarePropertyImplBase(id, info, backend)
{
}

outcome::result<void> SoftwarePropertyCommandImpl::execute()
{
    SPDLOG_WARN("Not implemented. {}", get_internal_name());
    return tcam::status::PropertyNotImplemented;
}

SoftwarePropertyEnumImpl::SoftwarePropertyEnumImpl(
    const std::shared_ptr<SoftwarePropertyBackend>& backend,
    software_prop id,
    const tcamprop1::prop_static_info_enumeration* info,
    std::vector<std::string_view>&& entries,
    int default_entry)
    : SoftwarePropertyImplBase(id, info, backend), m_entries(entries),
      m_default(m_entries[default_entry])
{
}

outcome::result<void> SoftwarePropertyEnumImpl::set_value(std::string_view new_value)
{
    for (size_t idx = 0; idx < m_entries.size(); ++idx)
    {
        if (m_entries[idx] == new_value)
        {
            if (auto ptr = m_cam.lock())
            {
                return ptr->set_int(m_id, static_cast<int64_t>(idx));
            }
            else
            {
                SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.",
                             get_internal_name());
                return tcam::status::ResourceNotLockable;
            }
        }
    }
    return tcam::status::PropertyValueOutOfBounds;
}

outcome::result<std::string_view> SoftwarePropertyEnumImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        OUTCOME_TRY(auto value, ptr->get_int(m_id));

        if (value < 0 || value >= static_cast<int64_t>(m_entries.size()))
        {
            return tcam::status::PropertyValueOutOfBounds;
        }

        return m_entries[value];
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot retrieve value.",
                     get_internal_name());
        return tcam::status::ResourceNotLockable;
    }
}

std::vector<std::string> SoftwarePropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    for (auto&& e : m_entries)
    {
        v.push_back(std::string(e));
    }
    return v;
}
