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

#include "v4l2_property_impl.h"

#include "../logging.h"
#include "../utils.h"
#include "V4L2PropertyBackend.h"
#include "v4l2_genicam_mapping.h"

#include <memory>

namespace
{

static void check_and_fixup_range(std::string_view prop_name,
                                  tcamprop1::prop_range_integer& range,
                                  int64_t def)
{
    if (range.stp == 0)
    {
        SPDLOG_DEBUG("Step size for property '{}' is 0.", prop_name, range.stp);
        range.stp = 1;
    }
    if (range.stp > 0)
    {
        if (range.min > range.max || def < range.min || def > range.max
            || (def - range.min) % range.stp != 0 /*|| range_.stp > (range_.max - range_.min)*/)
        {
            SPDLOG_DEBUG("Property '{}', invalid range. min={} max={} def={} stp={}.",
                         prop_name,
                         range.min,
                         range.max,
                         def,
                         range.stp);
        }
    }
}

static void check_and_fixup_range(std::string_view prop_name,
                                  const tcamprop1::prop_range_float& range,
                                  int64_t def)
{
    if (range.stp > 0)
    {
        if (range.min > range.max || def < range.min || def > range.max)
        {
            SPDLOG_DEBUG("Property '{}', invalid range. min={} max={} def={} stp={}.",
                         prop_name,
                         range.min,
                         range.max,
                         def,
                         range.stp);
        }
    }
}

} // namespace

tcam::property::V4L2PropertyImplBase::V4L2PropertyImplBase(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : m_name((const char*)queryctrl.name), m_v4l2_id(queryctrl.id), m_cam(backend)
{
}

tcam::property::V4L2PropertyImplBase::V4L2PropertyImplBase(
    const v4l2_queryctrl& queryctrl,
    const tcamprop1::prop_static_info* static_info,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : m_name(static_info->name), m_v4l2_id(queryctrl.id), m_cam(backend),
      p_static_info_base(static_info)
{
}

outcome::result<void> tcam::property::V4L2PropertyImplBase::set_backend_value(int64_t new_value)
{
    if (auto ptr = m_cam.lock())
    {
        OUTCOME_TRY(ptr->write_control(m_v4l2_id, new_value));
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
    return outcome::success();
}

outcome::result<int64_t> tcam::property::V4L2PropertyImplBase::get_backend_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->read_control(m_v4l2_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

tcam::property::V4L2PropertyIntegerImpl::V4L2PropertyIntegerImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_integer* static_info,
    tcam::v4l2::converter_scale scale)
    : V4L2PropertyImplBase(queryctrl, static_info, backend), m_converter(scale),
      p_static_info(static_info)
{
    range_ = { static_cast<int64_t>(m_converter.from_device(queryctrl.minimum)),
               static_cast<int64_t>(m_converter.from_device(queryctrl.maximum)),
               static_cast<int64_t>(m_converter.from_device(queryctrl.step)) };
    m_default = m_converter.from_device(queryctrl.default_value);

    check_and_fixup_range(get_internal_name(), range_, m_default);
}

tcam::property::V4L2PropertyIntegerImpl::V4L2PropertyIntegerImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
    range_ = { queryctrl.minimum, queryctrl.maximum, queryctrl.step };
    m_default = m_converter.from_device(queryctrl.default_value);

    check_and_fixup_range(get_internal_name(), range_, m_default);
}

std::string_view tcam::property::V4L2PropertyIntegerImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}

tcamprop1::IntRepresentation_t tcam::property::V4L2PropertyIntegerImpl::get_representation() const
{
    if (p_static_info)
    {
        return p_static_info->representation;
    }
    return tcamprop1::IntRepresentation_t::Linear;
}


outcome::result<int64_t> tcam::property::V4L2PropertyIntegerImpl::get_value() const
{
    auto ret = get_backend_value();
    if (ret.has_value())
    {
        return m_converter.from_device(ret.value());
    }
    return ret.error();
}

outcome::result<void> tcam::property::V4L2PropertyIntegerImpl::set_value(int64_t new_value)
{
    if (range_.min > new_value || new_value > range_.max)
    {
        return tcam::status::PropertyOutOfBounds;
    }
    if ((new_value % range_.stp) != 0)
    {
        SPDLOG_INFO("Value is not compatible with step size.");
        return tcam::status::PropertyOutOfBounds;
    }

    auto tmp = m_converter.to_device(new_value);
    return set_backend_value(tmp);
}

tcam::property::V4L2PropertyDoubleImpl::V4L2PropertyDoubleImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_float* static_info,
    tcam::v4l2::converter_scale scale)
    : V4L2PropertyImplBase(queryctrl, static_info, backend), m_converter(scale),
      p_static_info(static_info)
{
    range_ = { m_converter.from_device(queryctrl.minimum),
               m_converter.from_device(queryctrl.maximum),
               m_converter.from_device(queryctrl.step) };
    m_default = m_converter.from_device(queryctrl.default_value);

    check_and_fixup_range(get_internal_name(), range_, m_default);
}

tcam::property::V4L2PropertyDoubleImpl::V4L2PropertyDoubleImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
    range_ = { static_cast<double>(queryctrl.minimum),
               static_cast<double>(queryctrl.maximum),
               static_cast<double>(queryctrl.step) };
    m_default = static_cast<double>(queryctrl.default_value);

    check_and_fixup_range(get_internal_name(), range_, m_default);
}

std::string_view tcam::property::V4L2PropertyDoubleImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}

tcamprop1::FloatRepresentation_t tcam::property::V4L2PropertyDoubleImpl::get_representation() const
{
    if (p_static_info)
    {
        return p_static_info->representation;
    }
    return tcamprop1::FloatRepresentation_t::Linear;
}

outcome::result<double> tcam::property::V4L2PropertyDoubleImpl::get_value() const
{
    auto ret = get_backend_value();
    if (ret)
    {
        return m_converter.from_device(ret.value());
    }
    return ret.error();
}

outcome::result<void> tcam::property::V4L2PropertyDoubleImpl::set_value(double new_value)
{
    if (range_.min > new_value || new_value > range_.max)
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return set_backend_value(m_converter.to_device(new_value));
}

tcam::property::V4L2PropertyBoolImpl::V4L2PropertyBoolImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
    m_default = queryctrl.default_value != 0;
}

tcam::property::V4L2PropertyBoolImpl::V4L2PropertyBoolImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_boolean* static_info)
    : V4L2PropertyImplBase(queryctrl, static_info, backend), p_static_info(static_info)
{
    m_default = queryctrl.default_value != 0;
}

outcome::result<bool> tcam::property::V4L2PropertyBoolImpl::get_value() const
{
    auto ret = get_backend_value();
    if (ret)
    {
        return ret.value() != 0;
    }
    return ret.error();
}

outcome::result<void> tcam::property::V4L2PropertyBoolImpl::set_value(bool new_value)
{
    return set_backend_value(new_value != 0);
}

void tcam::property::V4L2PropertyBoolImpl::set_dependencies(
    std::vector<std::weak_ptr<PropertyLock>>& deps)
{
    m_dependencies = deps;

    // ensure that all dependencies are in the correct locked state
    // by explicitly setting them the the current value

    bool new_locked_state = lock_others();

    for (auto& dep : m_dependencies)
    {
        if (auto d = dep.lock())
        {
            d->set_locked(new_locked_state);
        }
    }
}

tcam::property::V4L2PropertyCommandImpl::V4L2PropertyCommandImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
}

tcam::property::V4L2PropertyCommandImpl::V4L2PropertyCommandImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_command* static_info)
    : V4L2PropertyImplBase(queryctrl, static_info, backend), p_static_info(static_info)
{
}

outcome::result<void> tcam::property::V4L2PropertyCommandImpl::execute()
{
    return set_backend_value(1);
}

tcam::property::V4L2PropertyEnumImpl::V4L2PropertyEnumImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
    m_entries = backend->get_menu_entries(queryctrl.id, queryctrl.maximum);

    m_default = get_entry_name(queryctrl.default_value);
}

tcam::property::V4L2PropertyEnumImpl::V4L2PropertyEnumImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_enumeration* static_info,
    tcam::v4l2::fetch_menu_entries_func func)
    : V4L2PropertyImplBase(queryctrl, static_info, backend), p_static_info(static_info)
{
    if (func)
    {
        m_entries = func();
    }
    else
    {
        m_entries = backend->get_menu_entries(queryctrl.id, queryctrl.maximum);
    }

    m_default = get_entry_name(queryctrl.default_value);
}

outcome::result<void> tcam::property::V4L2PropertyEnumImpl::set_value_str(
    const std::string_view& new_value)
{
    auto value_to_set = get_entry_value(new_value);
    if (value_to_set.has_error())
    {
        return value_to_set.error();
    }

    auto ret = set_backend_value(value_to_set.value());
    if (ret.has_error())
    {
        return ret.error();
    }

    if (!m_dependencies.empty())
    {
        bool new_locked_state = lock_others();
        for (auto& dep : m_dependencies)
        {
            if (auto d = dep.lock())
            {
                d->set_locked(new_locked_state);
            }
        }
    }

    return outcome::success();
}

outcome::result<std::string_view> tcam::property::V4L2PropertyEnumImpl::get_value() const
{
    OUTCOME_TRY(int64_t value, get_backend_value());

    for (const auto& [entry_value, entry_name] : m_entries)
    {
        if (entry_value == value)
        {
            return entry_name;
        }
    }
    return tcam::status::PropertyOutOfBounds;
}

std::vector<std::string> tcam::property::V4L2PropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    v.reserve(m_entries.size());
    for (const auto& [entry_value, entry_name] : m_entries) { v.push_back(entry_name); }
    return v;
}

void tcam::property::V4L2PropertyEnumImpl::set_dependencies(
    std::vector<std::weak_ptr<PropertyLock>>& deps)
{
    m_dependencies = deps;

    // ensure that all dependencies are in the correct locked state
    // by explicitly setting them the the current value

    bool new_locked_state = lock_others();

    for (auto& dep : m_dependencies)
    {
        if (auto d = dep.lock())
        {
            d->set_locked(new_locked_state);
        }
    }
}

std::string_view tcam::property::V4L2PropertyEnumImpl::get_entry_name(int value) const
{
    for (const auto& [entry_value, entry_name] : m_entries)
    {
        if (entry_value == value)
        {
            return entry_name;
        }
    }
    return {};
}

outcome::result<int64_t> tcam::property::V4L2PropertyEnumImpl::get_entry_value(
    std::string_view name) const
{
    for (const auto& [entry_value, entry_name] : m_entries)
    {
        if (entry_name == name)
        {
            return entry_value;
        }
    }
    return tcam::status::PropertyDoesNotExist;
}
