
#include "AFU420PropertyImpl.h"

#include "../logging.h"
#include "AFU420DeviceBackend.h"

namespace tcam::property
{


tcam::property::AFU420PropertyLockImpl::AFU420PropertyLockImpl(std::string_view name)
{
    dependency_info_ = tcam::property::find_dependency_entry(name);
}

std::vector<std::string_view> tcam::property::AFU420PropertyLockImpl::get_dependent_names() const
{
    if (dependency_info_)
    {
        return dependency_info_->dependent_property_names;
    }
    return {};
}

void tcam::property::AFU420PropertyLockImpl::update_dependent_lock_state()
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

void tcam::property::AFU420PropertyLockImpl::set_dependent_properties(
    std::vector<std::weak_ptr<PropertyLock>>&& controls)
{
    dependent_controls_ = std::move(controls);

    update_dependent_lock_state();
}

AFU420PropertyIntegerImpl::AFU420PropertyIntegerImpl(
    const std::string& name,
    tcam_value_int i,
    tcam::afu420::AFU420Property id,
    std::shared_ptr<tcam::property::AFU420DeviceBackend> cam)
    : AFU420PropertyLockImpl(name), m_cam(cam), m_name(name), m_id(id)
{

    m_default = i.default_value;
    m_min = i.min;
    m_max = i.max;
    m_step = i.step;

    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Integer && static_info.info_ptr)
    {
        p_static_info = static_cast<const tcamprop1::prop_static_info_integer*>(static_info.info_ptr);
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

std::string_view AFU420PropertyIntegerImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}


tcamprop1::IntRepresentation_t AFU420PropertyIntegerImpl::get_representation() const
{
    if (p_static_info)
    {
        return p_static_info->representation;
    }
    return tcamprop1::IntRepresentation_t::Linear;
}


outcome::result<int64_t> AFU420PropertyIntegerImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_int(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AFU420PropertyIntegerImpl::set_value(int64_t new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_int(m_id, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> AFU420PropertyIntegerImpl::valid_value(int64_t value)
{
    if (m_min > value || value > m_max)
    {
        return tcam::status::PropertyValueOutOfBounds;
    }

    return outcome::success();
}


AFU420PropertyDoubleImpl::AFU420PropertyDoubleImpl(
    const std::string& name,
    tcam_value_double d,
    tcam::afu420::AFU420Property id,
    std::shared_ptr<tcam::property::AFU420DeviceBackend> cam)
    : m_cam(cam), m_name(name), m_id(id)
{
    m_default = d.default_value;
    m_min = d.min;
    m_max = d.max;
    m_step = d.step;

    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

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

std::string_view AFU420PropertyDoubleImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}


tcamprop1::FloatRepresentation_t AFU420PropertyDoubleImpl::get_representation() const
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


outcome::result<double> AFU420PropertyDoubleImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_float(m_id);

        if (ret)
        {
            return ret.value();
        }
        return ret.as_failure();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AFU420PropertyDoubleImpl::set_value(double new_value)
{
    if (auto ptr = m_cam.lock())
    {
        OUTCOME_TRY(ptr->set_float(m_id, new_value));
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> AFU420PropertyDoubleImpl::valid_value(double value)
{
    if (m_min > value || value > m_max)
    {
        return tcam::status::PropertyValueOutOfBounds;
    }

    return outcome::success();
}


AFU420PropertyEnumImpl::AFU420PropertyEnumImpl(const std::string& name,
                                               tcam::afu420::AFU420Property id,
                                               std::map<int, std::string> entries,
                                               std::shared_ptr<AFU420DeviceBackend> backend)
    : AFU420PropertyLockImpl(name), m_entries(entries), m_cam(backend), m_name(name), m_id(id)
{
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

    // if (auto ptr = m_cam.lock())
    // {
    //     auto ret = ptr->get_int(m_ctrl, GET_DEF);
    //     if (ret)
    //     {
    //         m_default = m_entries.at(ret.value());
    //     }
    // }
    // else
    // {
    //     SPDLOG_ERROR("Unable to lock propertybackend. Cannot retrieve value.");
    // }
    auto static_info = tcamprop1::find_prop_static_info(m_name);

    if (static_info.type == tcamprop1::prop_type::Enumeration && static_info.info_ptr)
    {
        p_static_info = static_cast<const tcamprop1::prop_static_info_enumeration*>(static_info.info_ptr);
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

outcome::result<void> AFU420PropertyEnumImpl::set_value(std::string_view new_value)
{
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->second == new_value)
        {
            return set_value_int(it->first);
        }
    }
    return tcam::status::PropertyValueOutOfBounds;
}


outcome::result<void> AFU420PropertyEnumImpl::set_value_int(int64_t new_value)
{
    if (!valid_value(new_value))
    {
        return tcam::status::PropertyValueOutOfBounds;
    }

    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->set_int(m_id, new_value);
        if (ret.has_error())
        {
            return ret.error();
        }
        update_dependent_lock_state();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }

    return tcam::status::Success;
}


outcome::result<std::string_view> AFU420PropertyEnumImpl::get_value() const
{
    OUTCOME_TRY(auto value, get_value_int());

    // TODO: additional checks if key exists

    return m_entries.at(value);
}


outcome::result<int64_t> AFU420PropertyEnumImpl::get_value_int() const
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


std::vector<std::string> AFU420PropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) { v.push_back(it->second); }
    return v;
}


bool AFU420PropertyEnumImpl::valid_value(int value)
{
    auto it = m_entries.find(value);

    if (it == m_entries.end())
    {
        return false;
    }

    return true;
}


bool AFU420PropertyEnumImpl::should_set_dependent_locked() const
{
    auto dep_entry = get_dependency_entry();
    if (!dep_entry)
    {
        return false;
    }

    auto res = get_value();
    if (res.has_error())
    {
        return false;
    }

    return res.value() == dep_entry->prop_enum_state_for_locked;
}

} // namespace tcam::property
