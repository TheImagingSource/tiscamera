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
    const software_prop_desc& desc,
    std::shared_ptr<IPropertyInteger> prop,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    m_id = desc.id_;
    m_name = desc.name_;

    m_min = prop->get_min();
    m_max = prop->get_max();
    m_step = prop->get_step();
    m_default = prop->get_default();

    // do not add external flag
    // this is a wrapper around an existing property
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Integer && static_info.info_ptr)
    {
        p_static_info =
            static_cast<const tcamprop1::prop_static_info_integer*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}

SoftwarePropertyIntegerImpl::SoftwarePropertyIntegerImpl(
    const software_prop_desc& desc,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    m_id = desc.id_;

    m_name = desc.name_;

    m_min = desc.range_i_.min;
    m_max = desc.range_i_.max;
    m_step = desc.range_i_.step;
    m_default = desc.range_i_.default_value;
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented | PropertyFlags::External);

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Integer && static_info.info_ptr)
    {
        p_static_info =
            static_cast<const tcamprop1::prop_static_info_integer*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}

tcamprop1::prop_static_info SoftwarePropertyIntegerImpl::get_static_info() const
{
    if (p_static_info)
    {
        return *p_static_info;
    }
    return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
}

std::string_view SoftwarePropertyIntegerImpl::get_unit() const
{
    if (!p_static_info)
    {
        return std::string_view();
    }
    else
    {
        return p_static_info->unit;
    }
}

tcamprop1::IntRepresentation_t SoftwarePropertyIntegerImpl::get_representation() const
{
    if (p_static_info)
    {
        return p_static_info->representation;
    }
    return tcamprop1::IntRepresentation_t::Linear;
}


PropertyFlags SoftwarePropertyIntegerImpl::get_flags() const
{
    return m_flags;
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
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot read value.", m_name);
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
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.", m_name);
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
    const software_prop_desc& desc,
    std::shared_ptr<IPropertyFloat> prop,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_name(desc.name_), m_id(desc.id_), m_cam(backend)
{
    //m_name = prop->get_name();
    m_min = prop->get_min();
    m_max = prop->get_max();
    m_step = prop->get_step();
    m_default = prop->get_default();

    if (desc.device_flags)
    {
        m_device_flags = true;
    }
    else
    {
        // do not add external flag
        // this is a wrapper around an existing property
        m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
    }

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Float && static_info.info_ptr)
    {
        p_static_info = static_cast<const tcamprop1::prop_static_info_float*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}


SoftwarePropertyDoubleImpl::SoftwarePropertyDoubleImpl(
    const software_prop_desc& desc,
    std::shared_ptr<IPropertyInteger> prop,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_name(desc.name_), m_id(desc.id_), m_cam(backend)
{
    //m_name = prop->get_name();
    m_min = prop->get_min();
    m_max = prop->get_max();
    m_step = prop->get_step();
    m_default = prop->get_default();

    if (desc.device_flags)
    {
        m_device_flags = true;
    }
    else
    {
        // do not add external flag
        // this is a wrapper around an existing property
        m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
    }

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Float && static_info.info_ptr)
    {
        p_static_info = static_cast<const tcamprop1::prop_static_info_float*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}

SoftwarePropertyDoubleImpl::SoftwarePropertyDoubleImpl(
    const software_prop_desc& desc,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    m_id = desc.id_;

    m_name = desc.name_;

    m_min = desc.range_d_.min;
    m_max = desc.range_d_.max;
    m_step = desc.range_d_.step;
    m_default = desc.range_d_.default_value;

    if (desc.device_flags)
    {
        m_device_flags = true;
    }
    else
    {
        m_flags = (PropertyFlags::Available | PropertyFlags::Implemented | PropertyFlags::External);
    }
    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Float && static_info.info_ptr)
    {
        p_static_info = static_cast<const tcamprop1::prop_static_info_float*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}

tcamprop1::prop_static_info SoftwarePropertyDoubleImpl::get_static_info() const
{
    if (p_static_info)
    {
        return *p_static_info;
    }
    return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
}

std::string_view SoftwarePropertyDoubleImpl::get_unit() const
{
    if (!p_static_info)
    {
        return std::string_view();
    }
    else
    {
        return p_static_info->unit;
    }
}


tcamprop1::FloatRepresentation_t SoftwarePropertyDoubleImpl::get_representation() const
{

    if (p_static_info)
    {
        return p_static_info->representation;
    }
    else
    {
        return tcamprop1::FloatRepresentation_t::Linear;
    }
}


PropertyFlags SoftwarePropertyDoubleImpl::get_flags() const
{
    if (m_device_flags)
    {
        if (auto ptr = m_cam.lock())
        {
            return ptr->get_flags(m_id);
        }
        return tcam::property::PropertyFlags::None;
        //return tcam::status::ResourceNotLockable;
    }
    return m_flags;
}


outcome::result<double> SoftwarePropertyDoubleImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_double(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot read value.", m_name);
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
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.", m_name);
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


SoftwarePropertyBoolImpl::SoftwarePropertyBoolImpl(const software_prop_desc& desc,
                                                   std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    m_id = desc.id_;
    m_name = desc.name_;
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented | PropertyFlags::External);

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Boolean && static_info.info_ptr)
    {
        p_static_info =
            static_cast<const tcamprop1::prop_static_info_boolean*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}

tcamprop1::prop_static_info SoftwarePropertyBoolImpl::get_static_info() const
{
    if (p_static_info)
    {
        return *p_static_info;
    }
    return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
}

PropertyFlags SoftwarePropertyBoolImpl::get_flags() const
{
    return m_flags;
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

    SPDLOG_ERROR("Unable to lock property backend for {}. Cannot read value.", m_name);
    return tcam::status::ResourceNotLockable;
}

outcome::result<void> SoftwarePropertyBoolImpl::set_value(bool new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_int(m_id, new_value);
    }

    SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.", m_name);
    return tcam::status::ResourceNotLockable;
}


SoftwarePropertyCommandImpl::SoftwarePropertyCommandImpl(
    const software_prop_desc& desc,
    std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    m_name = desc.name_;
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Command && static_info.info_ptr)
    {
        p_static_info =
            static_cast<const tcamprop1::prop_static_info_command*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}


tcamprop1::prop_static_info SoftwarePropertyCommandImpl::get_static_info() const
{
    if (p_static_info)
    {
        return *p_static_info;
    }
    return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
}

PropertyFlags SoftwarePropertyCommandImpl::get_flags() const
{
    return m_flags;
}


outcome::result<void> SoftwarePropertyCommandImpl::execute()
{
    SPDLOG_WARN("Not implemented. {}", m_name);
    return tcam::status::NotImplemented;
}


SoftwarePropertyEnumImpl::SoftwarePropertyEnumImpl(const software_prop_desc& desc,
                                                   std::shared_ptr<SoftwarePropertyBackend> backend)
    : m_cam(backend)
{
    m_id = desc.id_;
    m_name = desc.name_;
    m_entries = desc.entries_;

    m_default = m_entries[desc.default_value_];
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented | PropertyFlags::External);

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Enumeration && static_info.info_ptr)
    {
        p_static_info =
            static_cast<const tcamprop1::prop_static_info_enumeration*>(static_info.info_ptr);
    }
    else if (!static_info.info_ptr)
    {
        SPDLOG_ERROR("static information for {} do not exist!", m_name);
        p_static_info = nullptr;
    }
    else
    {
        SPDLOG_ERROR("static information for {} have the wrong type!", m_name);
        p_static_info = nullptr;
    }
}

tcamprop1::prop_static_info SoftwarePropertyEnumImpl::get_static_info() const
{
    if (p_static_info)
    {
        return *p_static_info;
    }
    return tcamprop1::prop_static_info { /*.name =*/m_name, {}, {}, {} };
}

PropertyFlags SoftwarePropertyEnumImpl::get_flags() const
{
    return m_flags;
}


outcome::result<void> SoftwarePropertyEnumImpl::set_value_str(const std::string_view& new_value)
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
        return ptr->set_int(m_id, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot write value.", m_name);
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<std::string_view> SoftwarePropertyEnumImpl::get_value() const
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
        SPDLOG_ERROR("Unable to lock property backend for {}. Cannot retrieve value.", m_name);
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
