
#include "AFU050PropertyImpl.h"

#include "../logging.h"
#include "AFU050DeviceBackend.h"

namespace tcam::property
{


tcam::property::AFU050PropertyLockImpl::AFU050PropertyLockImpl(std::string_view name)
{
    dependency_info_ = tcam::property::find_dependency_entry(name);
}

std::vector<std::string_view> tcam::property::AFU050PropertyLockImpl::get_dependent_names() const
{
    if (dependency_info_)
    {
        return dependency_info_->dependent_property_names;
    }
    return {};
}

    void tcam::property::AFU050PropertyLockImpl::update_dependent_lock_state()
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

void tcam::property::AFU050PropertyLockImpl::set_dependent_properties(
    std::vector<std::weak_ptr<PropertyLock>>&& controls)
{
    dependent_controls_ = std::move(controls);

    update_dependent_lock_state();
}


AFU050PropertyIntegerImpl::AFU050PropertyIntegerImpl(const std::string& name, const tcam_value_int& val_def)
    : AFU050PropertyLockImpl(name), m_name(name)
{
    m_default = val_def.default_value;
    m_min = val_def.min;
    m_max = val_def.max;
    m_step = val_def.step;
    m_value= val_def.value;
    
    auto static_info = tcamprop1::find_prop_static_info(m_name);

    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

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


AFU050PropertyIntegerImpl::AFU050PropertyIntegerImpl(
    const std::string& name,
    control_definition ctrl,
    std::shared_ptr<tcam::property::AFU050DeviceBackend> cam)
    : AFU050PropertyLockImpl(name), m_cam(cam), m_name(name), m_ctrl(ctrl)
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_ctrl, GET_DEF);
        if (ret)
        {
            m_default = ret.value();
        }
        auto ret_min = ptr->get_int(m_ctrl, GET_MIN);
        if (ret_min)
        {
            m_min = ret_min.value();
        }
        auto ret_max = ptr->get_int(m_ctrl, GET_MAX);
        if (ret_max)
        {
            m_max = ret_max.value();
        }
        auto ret_step = ptr->get_int(m_ctrl, GET_RES);
        if (ret_step)
        {
            m_step = ret_step.value();
        }
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
    }

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

std::string_view AFU050PropertyIntegerImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}


tcamprop1::IntRepresentation_t AFU050PropertyIntegerImpl::get_representation() const
{
    if (p_static_info)
    {
        return p_static_info->representation;
    }
    return tcamprop1::IntRepresentation_t::Linear;
}


outcome::result<int64_t> AFU050PropertyIntegerImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_int(m_ctrl);
    }
    else
    {
        if (m_value != 0)
        {
            return m_value;
        }

        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AFU050PropertyIntegerImpl::set_value(int64_t new_value)
{
    if (auto ptr = m_cam.lock())
    {

         auto ret = ptr->set_int(m_ctrl, new_value);
         if (ret.has_error())
         {
             return ret.as_failure();
         }
         //update_dependent_lock_state();

         return tcam::status::Success;

    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


// bool AFU050PropertyIntegerImpl::should_set_dependent_locked() const
// {
//     auto dep_entry = get_dependency_entry();
//     if (!dep_entry)
//     {
//         return false;
//     }

//     auto res = get_value();
//     if (res.has_error())
//     {
//         return false;
//     }

//     return res.value() == dep_entry->prop_enum_state_for_locked;
// }


AFU050PropertyDoubleImpl::AFU050PropertyDoubleImpl(
    const std::string& name,
    control_definition ctrl,
    std::shared_ptr<tcam::property::AFU050DeviceBackend> cam,
    double modifier)
    : AFU050PropertyLockImpl(name), m_cam(cam), m_name(name), m_modifier(modifier), m_ctrl(ctrl)
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_ctrl, GET_DEF);
        if (ret)
        {
            m_default = ret.value() / modifier;
        }
        auto ret_min = ptr->get_int(m_ctrl, GET_MIN);
        if (ret_min)
        {
            m_min = ret_min.value() / modifier;
        }
        auto ret_max = ptr->get_int(m_ctrl, GET_MAX);
        if (ret_max)
        {
            m_max = ret_max.value() / modifier;
        }
        auto ret_step = ptr->get_int(m_ctrl, GET_RES);
        if (ret_step)
        {
            m_step = ret_step.value() / modifier;
        }
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
    }
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

std::string_view AFU050PropertyDoubleImpl::get_unit() const
{
    if (p_static_info)
    {
        return p_static_info->unit;
    }
    return std::string_view();
}


tcamprop1::FloatRepresentation_t AFU050PropertyDoubleImpl::get_representation() const
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


outcome::result<double> AFU050PropertyDoubleImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_ctrl);

        if (ret)
        {
            return ret.value() / m_modifier;
        }
        return ret.as_failure();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AFU050PropertyDoubleImpl::set_value(double new_value)
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->set_int(m_ctrl, new_value * m_modifier);

        if (ret.has_error())
        {
            return ret.as_failure();
        }

        //update_dependent_lock_state();

        return tcam::status::Success;
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> AFU050PropertyDoubleImpl::valid_value(double value)
{
    if (m_min > value || value > m_max)
    {
        return tcam::status::PropertyValueOutOfBounds;
    }

    return outcome::success();
}


// bool AFU050PropertyDoubleImpl::should_set_dependent_locked() const
// {
//     auto dep_entry = get_dependency_entry();
//     if (!dep_entry)
//     {
//         return false;
//     }

//     auto res = get_value();
//     if (res.has_error())
//     {
//         return false;
//     }

//     return res.value() == dep_entry->prop_enum_state_for_locked;
// }


AFU050PropertyEnumImpl::AFU050PropertyEnumImpl(const std::string& name,
                                               control_definition ctrl,
                                               std::map<int, std::string> entries,
                                               std::shared_ptr<AFU050DeviceBackend> backend)
    : AFU050PropertyLockImpl(name), m_entries(entries), m_cam(backend), m_name(name), m_ctrl(ctrl)
{
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);

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

    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_ctrl, GET_DEF);
        if (ret)
        {
            m_default = m_entries.at(ret.value());
        }
    }
    else
    {
        SPDLOG_ERROR("Unable to lock propertybackend. Cannot retrieve value.");
    }
}

outcome::result<void> AFU050PropertyEnumImpl::set_value(std::string_view new_value)
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


outcome::result<void> AFU050PropertyEnumImpl::set_value_int(int64_t new_value)
{
    if (!valid_value(new_value))
    {
        return tcam::status::PropertyValueOutOfBounds;
    }

    if (auto ptr = m_cam.lock())
    {
        if (!ptr->set_int(m_ctrl, new_value))
        {
            SPDLOG_ERROR("Something went wrong while writing {}", m_name);
            return tcam::status::ResourceNotLockable;
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


outcome::result<std::string_view> AFU050PropertyEnumImpl::get_value() const
{
    OUTCOME_TRY(auto value, get_value_int());

    // TODO: additional checks if key exists

    return m_entries.at(value);
}


outcome::result<int64_t> AFU050PropertyEnumImpl::get_value_int() const
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->get_int(m_ctrl);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock propertybackend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


std::vector<std::string> AFU050PropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) { v.push_back(it->second); }
    return v;
}


bool AFU050PropertyEnumImpl::valid_value(int value)
{
    auto it = m_entries.find(value);

    if (it == m_entries.end())
    {
        return false;
    }

    return true;
}


bool AFU050PropertyEnumImpl::should_set_dependent_locked() const
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
