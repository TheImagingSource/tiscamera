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

#include "SoftwarePropertyBackend.h"
#include "logging.h"

using namespace tcam::property::emulated;
namespace tcam::property::emulated
{

SoftwarePropertyIntegerImpl::SoftwarePropertyIntegerImpl(
    const struct software_prop_desc* desc,
    std::shared_ptr<IPropertyInteger> prop,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    m_id = desc->id_;

    m_name = desc->name_;

    m_min = prop->get_min();
    m_max = prop->get_max();
    m_step = prop->get_step();
    m_default = prop->get_default();

    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}

SoftwarePropertyIntegerImpl::SoftwarePropertyIntegerImpl(
    const struct software_prop_desc* desc,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    if (desc)
    {
        m_id = desc->id_;

        m_name = desc->name_;

        m_min = desc->range_i_.min;
        m_max = desc->range_i_.max;
        m_step = desc->range_i_.step;
        m_default = desc->range_i_.default_value;
    }
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}

outcome::result<int64_t> SoftwarePropertyIntegerImpl::get_value() const
{
    int64_t value = 0;

    if (auto ptr = m_cam.lock())
    {
        return ptr->get_int(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }

    return value;
}


outcome::result<void> SoftwarePropertyIntegerImpl::set_value(int64_t new_value)
{
    OUTCOME_TRY(valid_value(new_value));

    if (auto ptr = m_cam.lock())
    {
        if (ptr->set_int(m_id, new_value))
        {
            return tcam::status::Success;
        }
        return tcam::status::UndefinedError;
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> SoftwarePropertyIntegerImpl::valid_value(int64_t val)
{
    if (m_max < val || m_min > val)
    {
        return tcam::status::PropertyOutOfBounds;
    }
    if (val % m_step != 0)
    {
        return tcam::status::PropertyValueDoesNotExist;
    }

    return outcome::success();
}


SoftwarePropertyDoubleImpl::SoftwarePropertyDoubleImpl(
    const struct software_prop_desc* desc,
    std::shared_ptr<IPropertyFloat> prop,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_name(desc->name_), m_id(desc->id_), m_cam(backend)
{
    //m_name = prop->get_name();
    m_min = prop->get_min();
    m_max = prop->get_max();
    m_step = prop->get_step();
    m_default = prop->get_default();

    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}

SoftwarePropertyDoubleImpl::SoftwarePropertyDoubleImpl(
    const struct software_prop_desc* desc,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    if (desc)
    {
        m_id = desc->id_;

        m_name = desc->name_;

        m_min = desc->range_d_.min;
        m_max = desc->range_d_.max;
        m_step = desc->range_d_.step;
        m_default = desc->range_d_.default_value;

        m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
        //m_value = 500.0;
    }
}


outcome::result<double> SoftwarePropertyDoubleImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_double(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> SoftwarePropertyDoubleImpl::set_value(double new_value)
{
    OUTCOME_TRY(valid_value(new_value));

    if (auto ptr = m_cam.lock())
    {
        return ptr->set_double(m_id, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> SoftwarePropertyDoubleImpl::valid_value(double val)
{
    if (get_min() > val || val > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


SoftwarePropertyBoolImpl::SoftwarePropertyBoolImpl(const struct software_prop_desc* desc,
                                                   std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    if (desc)
    {
        m_id = desc->id_;
        m_name = desc->name_;
    }
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

}

outcome::result<bool> SoftwarePropertyBoolImpl::get_value() const
{
    SPDLOG_WARN("Not implemented. {}", m_name);
    return false;
}

outcome::result<void> SoftwarePropertyBoolImpl::set_value(bool new_value)
{
    SPDLOG_WARN("Not implemented set_value. {} {}", m_name, new_value);

    return tcam::status::NotImplemented;
}


SoftwarePropertyCommandImpl::SoftwarePropertyCommandImpl(
    const struct software_prop_desc* desc,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    if (desc)
    {
        m_name = desc->name_;
    }
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}


outcome::result<void> SoftwarePropertyCommandImpl::execute()
{
    SPDLOG_WARN("Not implemented. {}", m_name);
    return tcam::status::NotImplemented;
}


SoftwarePropertyEnumImpl::SoftwarePropertyEnumImpl(const struct software_prop_desc* desc,
                                                   std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    if (desc)
    {
        m_id = desc->id_;
        m_name = desc->name_;
        m_entries = desc->entries_;

        m_default = m_entries[desc->default_value_];
    }
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}

outcome::result<void> SoftwarePropertyEnumImpl::set_value_str(const std::string& new_value)
{
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->second == new_value)
        {
            return set_value(it->first);
        }
    }
    return tcam::status::PropertyValueDoesNotExist;
}

outcome::result<void> SoftwarePropertyEnumImpl::set_value(int64_t new_value)
{
    if (!valid_value(new_value))
    {
        return tcam::status::PropertyValueDoesNotExist;
    }

    if (auto ptr = m_cam.lock())
    {
        if (!ptr->set_int(m_id, new_value))
        {
            SPDLOG_ERROR("Something went wrong while writing {}", m_name);
            return tcam::status::ResourceNotLockable;
        }
        else
        {
            //SPDLOG_DEBUG("Wrote {} {}", m_name, new_value);
        }
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }

    return tcam::status::Success;
}

outcome::result<std::string> SoftwarePropertyEnumImpl::get_value() const
{
    OUTCOME_TRY(auto value, get_value_int());

    // TODO: additional checks if key exists

    return m_entries.at(value);
}

outcome::result<int64_t> SoftwarePropertyEnumImpl::get_value_int() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_int(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock propertybackend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

std::vector<std::string> SoftwarePropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) { v.push_back(it->second); }
    return v;
}

bool SoftwarePropertyEnumImpl::valid_value(int value)
{
    auto it = m_entries.find(value);

    if (it == m_entries.end())
    {
        return false;
    }

    return true;
}

} // namespace tcam::property::emulated
