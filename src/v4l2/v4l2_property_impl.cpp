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
#include "V4L2PropertyBackend.h"

#include <tcamprop1.0_base/tcamprop_property_info_list.h>

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


outcome::result<void> tcam::v4l2::V4L2PropertyBackendWrapper::set_backend_value(int64_t new_value)
{
    return set_backend_value(v4l2_id_, new_value);
}

outcome::result<void> tcam::v4l2::V4L2PropertyBackendWrapper::set_backend_value(uint32_t ctrl_id,
                                                                                int64_t new_value)
{
    if (auto ptr = device_ptr_.lock())
    {
        OUTCOME_TRY(ptr->write_control(ctrl_id, new_value));
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
    return outcome::success();
}

outcome::result<int64_t> tcam::v4l2::V4L2PropertyBackendWrapper::get_backend_value() const
{
    return get_backend_value(v4l2_id_);
}

outcome::result<int64_t> tcam::v4l2::V4L2PropertyBackendWrapper::get_backend_value(
    uint32_t ctrl_id) const
{
    if (auto ptr = device_ptr_.lock())
    {
        return ptr->read_control(ctrl_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

tcam::v4l2::V4L2PropertyLockImpl::V4L2PropertyLockImpl(std::string_view name)
{
    dependency_info_ = tcam::property::find_dependency_entry(name);
}

std::vector<std::string_view> tcam::v4l2::V4L2PropertyLockImpl::get_dependent_names() const
{
    if (dependency_info_)
    {
        return dependency_info_->dependent_property_names;
    }
    return {};
}

void tcam::v4l2::V4L2PropertyLockImpl::update_dependent_lock_state()
{
    if (dependent_controls_.empty())
        return;

    bool new_locked_state = should_set_dependent_locked();
    for (auto& dep : dependent_controls_)
    {
        if (auto d = dep.lock())
        {
            d->set_locked(new_locked_state);
        }
    }
}

void tcam::v4l2::V4L2PropertyLockImpl::set_dependent_properties(
    std::vector<std::weak_ptr<PropertyLock>>&& controls)
{
    dependent_controls_ = std::move(controls);

    update_dependent_lock_state();
}

tcam::v4l2::V4L2PropertyIntegerImpl::V4L2PropertyIntegerImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_integer* static_info,
    tcam::v4l2::converter_scale scale)
    : V4L2PropertyImplBase(queryctrl, static_info, backend), m_converter(scale),
      p_static_info(static_info)
{
    if (scale.overwrite_max_.has_value())
        range_.max = static_cast<int64_t>(scale.overwrite_max_.value());
    else
        range_.max = static_cast<int64_t>(m_converter.from_device(queryctrl.maximum));

    if (scale.overwrite_min_.has_value())
        range_.min = static_cast<int64_t>(scale.overwrite_min_.value());
    else
        range_.min = static_cast<int64_t>(m_converter.from_device(queryctrl.minimum));

    if (scale.overwrite_stp_.has_value())
        range_.stp = static_cast<int64_t>(scale.overwrite_stp_.value());
    else
        range_.stp = static_cast<int64_t>(m_converter.from_device(queryctrl.step));

    if (scale.overwrite_def_.has_value())
        m_default = static_cast<int64_t>(scale.overwrite_def_.value());
    else
        m_default = static_cast<int64_t>(m_converter.from_device(queryctrl.default_value));

    check_and_fixup_range(get_internal_name(), range_, m_default);
}

tcam::v4l2::V4L2PropertyIntegerImpl::V4L2PropertyIntegerImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
    range_ = { queryctrl.minimum, queryctrl.maximum, queryctrl.step };
    m_default = m_converter.from_device(queryctrl.default_value);

    check_and_fixup_range(get_internal_name(), range_, m_default);
}

std::string_view tcam::v4l2::V4L2PropertyIntegerImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}

tcamprop1::IntRepresentation_t tcam::v4l2::V4L2PropertyIntegerImpl::get_representation() const
{
    if (p_static_info)
    {
        return p_static_info->representation;
    }
    return tcamprop1::IntRepresentation_t::Linear;
}


outcome::result<int64_t> tcam::v4l2::V4L2PropertyIntegerImpl::get_value() const
{
    auto ret = backend_.get_backend_value();
    if (ret.has_value())
    {
        return m_converter.from_device(ret.value());
    }
    return ret.error();
}

outcome::result<void> tcam::v4l2::V4L2PropertyIntegerImpl::set_value(int64_t new_value)
{
    if (range_.min > new_value || new_value > range_.max)
    {
        SPDLOG_DEBUG("Property '{}', value of {} is not in range of [{},{}].",
                     get_internal_name(),
                     new_value,
                     range_.min,
                     range_.max);
        return tcam::status::PropertyOutOfBounds;
    }
    if ((new_value % range_.stp) != 0)
    {
        SPDLOG_DEBUG("Property '{}', value of {} is incompatible with step size of {}.",
                     get_internal_name(),
                     new_value,
                     range_.stp);
        return tcam::status::PropertyOutOfBounds;
    }

    auto tmp = m_converter.to_device(new_value);
    return backend_.set_backend_value(tmp);
}

tcam::v4l2::V4L2PropertyDoubleImpl::V4L2PropertyDoubleImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_float* static_info,
    tcam::v4l2::converter_scale scale)
    : V4L2PropertyImplBase(queryctrl, static_info, backend), m_converter(scale),
      p_static_info(static_info)
{
    if (scale.overwrite_max_.has_value())
        range_.max = scale.overwrite_max_.value();
    else
        range_.max = m_converter.from_device(queryctrl.maximum);

    if (scale.overwrite_min_.has_value())
        range_.min = scale.overwrite_min_.value();
    else
        range_.min = m_converter.from_device(queryctrl.minimum);

    if (scale.overwrite_stp_.has_value())
        range_.stp = scale.overwrite_stp_.value();
    else
        range_.stp = m_converter.from_device(queryctrl.step);

    if (scale.overwrite_def_.has_value())
        m_default = scale.overwrite_def_.value();
    else
        m_default = m_converter.from_device(queryctrl.default_value);

    check_and_fixup_range(get_internal_name(), range_, m_default);
}

tcam::v4l2::V4L2PropertyDoubleImpl::V4L2PropertyDoubleImpl(
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

std::string_view tcam::v4l2::V4L2PropertyDoubleImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}

tcamprop1::FloatRepresentation_t tcam::v4l2::V4L2PropertyDoubleImpl::get_representation() const
{
    if (p_static_info)
    {
        return p_static_info->representation;
    }
    return tcamprop1::FloatRepresentation_t::Linear;
}

outcome::result<double> tcam::v4l2::V4L2PropertyDoubleImpl::get_value() const
{
    auto ret = backend_.get_backend_value();
    if (ret)
    {
        return m_converter.from_device(ret.value());
    }
    return ret.error();
}

outcome::result<void> tcam::v4l2::V4L2PropertyDoubleImpl::set_value(double new_value)
{
    if (range_.stp >= 0 && (new_value < range_.min || new_value > range_.max))
    {
        if( new_value < range_.min && (new_value + range_.stp) >= range_.min )
        { // if new_value is slightly smaller then range_.min, we assume a float/double rounding error and just use min
            new_value = range_.min;
        }
        else if( new_value > range_.max && (new_value - range_.stp) <= range_.max )
        {// if new_value is slightly larger then range_.max, we assume a float/double rounding error and just use max
            new_value = range_.max;
        }
        else
        {
            SPDLOG_DEBUG("Property '{}', value of {} is not in range of [{},{}].",
                        get_internal_name(),
                        new_value,
                        range_.min,
                        range_.max);
            return tcam::status::PropertyOutOfBounds;
        }
    }

    return backend_.set_backend_value(m_converter.to_device(new_value));
}

tcam::v4l2::V4L2PropertyBoolImpl::V4L2PropertyBoolImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
    m_default = queryctrl.default_value != 0;
}

tcam::v4l2::V4L2PropertyBoolImpl::V4L2PropertyBoolImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_boolean* static_info)
    : V4L2PropertyImplBase(queryctrl, static_info, backend)
{
    m_default = queryctrl.default_value != 0;
}

outcome::result<bool> tcam::v4l2::V4L2PropertyBoolImpl::get_value() const
{
    auto ret = backend_.get_backend_value();
    if (ret)
    {
        return ret.value() != 0;
    }
    return ret.error();
}

outcome::result<void> tcam::v4l2::V4L2PropertyBoolImpl::set_value(bool new_value)
{
    return backend_.set_backend_value(new_value != 0);
}

tcam::v4l2::V4L2PropertyCommandImpl::V4L2PropertyCommandImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
}

tcam::v4l2::V4L2PropertyCommandImpl::V4L2PropertyCommandImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend,
    const tcamprop1::prop_static_info_command* static_info)
    : V4L2PropertyImplBase(queryctrl, static_info, backend)
{
}

outcome::result<void> tcam::v4l2::V4L2PropertyCommandImpl::execute()
{
    return backend_.set_backend_value(1);
}

tcam::v4l2::V4L2PropertyEnumImpl::V4L2PropertyEnumImpl(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyImplBase(queryctrl, backend)
{
    m_entries = backend->get_menu_entries(queryctrl.id, queryctrl.maximum);

    m_default = get_entry_name(queryctrl.default_value);
}

tcam::v4l2::V4L2PropertyEnumImpl::V4L2PropertyEnumImpl(
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

outcome::result<void> tcam::v4l2::V4L2PropertyEnumImpl::set_value_str(
    const std::string_view& new_value)
{
    auto value_to_set = get_entry_value(new_value);
    if (value_to_set.has_error())
    {
        SPDLOG_DEBUG(
            "Property '{}', value of {} is not in enumeration.", get_internal_name(), new_value);
        return value_to_set.error();
    }

    auto ret = backend_.set_backend_value(value_to_set.value());
    if (ret.has_error())
    {
        return ret.error();
    }

    update_dependent_lock_state();

    return outcome::success();
}

outcome::result<std::string_view> tcam::v4l2::V4L2PropertyEnumImpl::get_value() const
{
    OUTCOME_TRY(int64_t value, backend_.get_backend_value());

    for (const auto& [entry_value, entry_name] : m_entries)
    {
        if (entry_value == value)
        {
            return entry_name;
        }
    }
    return tcam::status::PropertyOutOfBounds;
}

std::vector<std::string> tcam::v4l2::V4L2PropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    v.reserve(m_entries.size());
    for (const auto& [entry_value, entry_name] : m_entries) { v.push_back(entry_name); }
    return v;
}

std::string_view tcam::v4l2::V4L2PropertyEnumImpl::get_entry_name(int value) const
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

outcome::result<int64_t> tcam::v4l2::V4L2PropertyEnumImpl::get_entry_value(
    std::string_view name) const
{
    for (const auto& [entry_value, entry_name] : m_entries)
    {
        if (entry_name == name)
        {
            return entry_value;
        }
    }
    return tcam::status::PropertyOutOfBounds;
}

bool tcam::v4l2::V4L2PropertyEnumImpl::should_set_dependent_locked() const
{
    auto dep_entry = get_dependency_entry();
    if (!dep_entry)
        return false;

    auto res = get_value();
    if (res.has_error())
        return false;

    return res.value() == dep_entry->prop_enum_state_for_locked;
}

static auto fetch_balance_white_entries()
{
    using namespace tcam::v4l2;
    return std::vector<menu_entry> {
        menu_entry { 0, "Off" },
        menu_entry { 1, "Continuous" },
        menu_entry { 2, "Once" },
    };
}

tcam::v4l2::prop_impl_33U_balance_white_auto::prop_impl_33U_balance_white_auto(
    const v4l2_queryctrl& queryctrl,
    const std::shared_ptr<V4L2PropertyBackend>& backend)
    : V4L2PropertyEnumImpl(queryctrl,
                           backend,
                           &tcamprop1::prop_list::BalanceWhiteAuto,
                           fetch_balance_white_entries)
{
}

outcome::result<void> tcam::v4l2::prop_impl_33U_balance_white_auto::set_value_str(
    const std::string_view& new_value)
{
    if (new_value == "Once")
    {
        return backend_.set_backend_value(V4L2_CID_TIS_WHITEBALANCE_ONE_PUSH, 1);
    }
    return V4L2PropertyEnumImpl::set_value_str(new_value);
}
