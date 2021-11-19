
#include "AFU050PropertyImpl.h"

#include "../logging.h"
#include "AFU050DeviceBackend.h"

namespace tcam::property
{

AFU050PropertyIntegerImpl::AFU050PropertyIntegerImpl(
    const std::string& name,
    control_definition ctrl,
    std::shared_ptr<tcam::property::AFU050DeviceBackend> cam)
    : m_cam(cam), m_name(name), m_ctrl(ctrl)
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_ctrl, GET_DEF);
        if (ret)
        {
            m_default = ret.value();
        }
        auto ret_min = ptr->get_int(m_ctrl, GET_MIN);
        if (ret)
        {
            m_min = ret_min.value();
        }
        auto ret_max = ptr->get_int(m_ctrl, GET_MAX);
        if (ret)
        {
            m_max = ret_max.value();
        }
        auto ret_step = ptr->get_int(m_ctrl, GET_RES);
        if (ret)
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
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AFU050PropertyIntegerImpl::set_value(int64_t new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_int(m_ctrl, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> AFU050PropertyIntegerImpl::valid_value(int64_t value)
{
    if (get_min() > value || value > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


AFU050PropertyDoubleImpl::AFU050PropertyDoubleImpl(
    const std::string& name,
    control_definition ctrl,
    std::shared_ptr<tcam::property::AFU050DeviceBackend> cam)
    : m_cam(cam), m_name(name), m_ctrl(ctrl)
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->get_int(m_ctrl, GET_DEF);
        if (ret)
        {
            m_default = ret.value();
        }
        auto ret_min = ptr->get_int(m_ctrl, GET_MIN);
        if (ret)
        {
            m_min = ret_min.value();
        }
        auto ret_max = ptr->get_int(m_ctrl, GET_MAX);
        if (ret)
        {
            m_max = ret_max.value();
        }
        auto ret_step = ptr->get_int(m_ctrl, GET_RES);
        if (ret)
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

outcome::result<void> AFU050PropertyDoubleImpl::set_value(double new_value)
{
    if (auto ptr = m_cam.lock())
    {
        return ptr->set_int(m_ctrl, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> AFU050PropertyDoubleImpl::valid_value(double value)
{
    if (get_min() > value || value > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


AFU050PropertyEnumImpl::AFU050PropertyEnumImpl(const std::string& name,
                                               control_definition ctrl,
                                               std::map<int, std::string> entries,
                                               std::shared_ptr<AFU050DeviceBackend> backend)
    : m_entries(entries), m_cam(backend), m_name(name), m_ctrl(ctrl)
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

outcome::result<void> AFU050PropertyEnumImpl::set_value_str(const std::string_view& new_value)
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


outcome::result<void> AFU050PropertyEnumImpl::set_value(int64_t new_value)
{
    if (!valid_value(new_value))
    {
        return tcam::status::PropertyValueDoesNotExist;
    }

    if (auto ptr = m_cam.lock())
    {
        if (!ptr->set_int(m_ctrl, new_value))
        {
            SPDLOG_ERROR("Something went wrong while writing {}", m_name);
            return tcam::status::ResourceNotLockable;
        }
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


} // namespace tcam::property
