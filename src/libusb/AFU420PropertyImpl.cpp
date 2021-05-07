
#include "AFU420PropertyImpl.h"

#include "AFU420DeviceBackend.h"
#include "logging.h"

namespace tcam::property
{

AFU420PropertyIntegerImpl::AFU420PropertyIntegerImpl(
    const std::string& name,
    tcam_value_int i,
    tcam::afu420::AFU420Property id,
    std::shared_ptr<tcam::property::AFU420DeviceBackend> cam)
    : m_cam(cam), m_name(name), m_id(id)
{

    m_default = i.default_value;
    m_min = i.min;
    m_max = i.max;
    m_step = i.step;

    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
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
    if (get_min() > value || value > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
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
}


outcome::result<double> AFU420PropertyDoubleImpl::get_value() const
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
        OUTCOME_TRY(ptr->set_int(m_id, new_value));
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
    if (get_min() > value || value > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


AFU420PropertyBoolImpl::AFU420PropertyBoolImpl(
    const std::string& name,
    bool default_value,
    tcam::afu420::AFU420Property id,
    std::shared_ptr<tcam::property::AFU420DeviceBackend> backend)
    : m_name(name), m_cam(backend), m_default(default_value), m_id(id)
{
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
    m_value = m_default;
}


outcome::result<bool> AFU420PropertyBoolImpl::get_value() const
{
    // if (m_ctrl.is_write_only)
    // {
    //     return m_value;
    // }

    if (auto ptr = m_cam.lock())
    {
        return ptr->get_bool(m_id);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> AFU420PropertyBoolImpl::set_value(bool new_value)
{
    //unsigned short value = new_value ? 0xFFFF : 0x0;
    if (auto ptr = m_cam.lock())
    {
        if (ptr->set_bool(m_id, new_value))
        {
            m_value = new_value;
            return outcome::success();
        }
        return tcam::status::UndefinedError;
    }
    else
    {
        SPDLOG_ERROR("Unable to lock property backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


AFU420PropertyEnumImpl::AFU420PropertyEnumImpl(const std::string& name,
                                               tcam::afu420::AFU420Property id,
                                               std::map<int, std::string> entries,
                                               std::shared_ptr<AFU420DeviceBackend> backend)
    : m_entries(entries), m_cam(backend), m_name(name), m_id(id)
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
}


outcome::result<void> AFU420PropertyEnumImpl::set_value_str(const std::string& new_value)
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


outcome::result<void> AFU420PropertyEnumImpl::set_value(int64_t new_value)
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
        SPDLOG_ERROR("Unable to lock property backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }

    return tcam::status::Success;
}


outcome::result<std::string> AFU420PropertyEnumImpl::get_value() const
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


} // namespace tcam::property
