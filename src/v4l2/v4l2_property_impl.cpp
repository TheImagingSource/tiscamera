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
#include "v4l2_genicam_mapping.h"

#include <memory>

namespace tcam::property
{

V4L2PropertyIntegerImpl::V4L2PropertyIntegerImpl(struct v4l2_queryctrl* queryctrl,
                                                 struct v4l2_ext_control* ctrl,
                                                 std::shared_ptr<V4L2PropertyBackend> backend,
                                                 const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    m_v4l2_id = queryctrl->id;
    m_name = (char*)queryctrl->name;

    m_converter = {[](double val){return val;}, [](double val){return val;}};
    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            m_name = mapping->gen_name;
        }
        if (mapping->conversion_type == tcam::v4l2::MappingType::Scale)
        {
            auto tmp = tcam::v4l2::find_scale(queryctrl->id);

            if (tmp.from_device && tmp.to_device)
            {
                m_converter = tmp;
            }
        }
    }

    m_min = m_converter.from_device(queryctrl->minimum);
    m_max = m_converter.from_device(queryctrl->maximum);
    m_step = m_converter.from_device(queryctrl->step);

    if (m_step == 0)
    {
        m_step = 1;
    }

    m_default = m_converter.from_device(ctrl->value);

    m_cam = backend;
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}


outcome::result<int64_t> V4L2PropertyIntegerImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->read_control(m_v4l2_id);
        if (ret)
        {
            return m_converter.from_device(ret.value());
        }
        else
        {
            return ret.as_failure();
        }
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> V4L2PropertyIntegerImpl::set_value(int64_t new_value)
{
    OUTCOME_TRY(valid_value(new_value));

    if (auto ptr = m_cam.lock())
    {
        auto tmp = m_converter.to_device(new_value);
        OUTCOME_TRY(ptr->write_control(m_v4l2_id, tmp));
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }

    return outcome::success();
}


outcome::result<void> V4L2PropertyIntegerImpl::valid_value(int64_t val)
{
    if (val % get_step() != 0)
    {
        return tcam::status::PropertyValueDoesNotExist;
    }

    if (get_min() > val || val > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


V4L2PropertyDoubleImpl::V4L2PropertyDoubleImpl(struct v4l2_queryctrl* queryctrl,
                                               struct v4l2_ext_control* ctrl,
                                               std::shared_ptr<V4L2PropertyBackend> backend,
                                               const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    m_name = (char*)queryctrl->name;
    m_v4l2_id = queryctrl->id;

    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            m_name = mapping->gen_name;
        }
        if (mapping->conversion_type == tcam::v4l2::MappingType::IntToDouble)
        {
            m_converter = tcam::v4l2::find_int_to_double(m_v4l2_id);
        }
    }


    m_min = conv_double(queryctrl->minimum);
    m_max = conv_double(queryctrl->maximum);
    m_step = conv_double(queryctrl->step);
    m_default = conv_double(ctrl->value);

    m_cam = backend;
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}


outcome::result<double> V4L2PropertyDoubleImpl::get_value() const
{

    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->read_control(m_v4l2_id);
        if (ret)
        {
            return conv_double(ret.value());
        }
        return ret.as_failure();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> V4L2PropertyDoubleImpl::set_value(double new_value)
{
    OUTCOME_TRY(valid_value(new_value));

    if (auto ptr = m_cam.lock())
    {
        auto r = ptr->write_control(m_v4l2_id, conv_int(new_value));
        if (!r)
        {
            return r.as_failure();
        }
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<void> V4L2PropertyDoubleImpl::valid_value(double val)
{
    if (get_min() > val || val > get_max())
    {
        return tcam::status::PropertyOutOfBounds;
    }

    return outcome::success();
}


V4L2PropertyBoolImpl::V4L2PropertyBoolImpl(struct v4l2_queryctrl* queryctrl,
                                           struct v4l2_ext_control* ctrl,
                                           std::shared_ptr<V4L2PropertyBackend> backend,
                                           const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    if (ctrl->value == 0)
    {
        m_default = false;
    }
    else
    {
        m_default = true;
    }
    m_name = (char*)queryctrl->name;
    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            m_name = mapping->gen_name;
        }
    }

    m_v4l2_id = queryctrl->id;

    m_cam = backend;
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}


outcome::result<bool> V4L2PropertyBoolImpl::get_value() const
{
    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->read_control(m_v4l2_id);
        if (ret)
        {
            return ret.value();
        }
        return ret.error();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return tcam::status::ResourceNotLockable;
    }
}

outcome::result<void> V4L2PropertyBoolImpl::set_value(bool new_value)
{
    int64_t val = 0;
    if (new_value)
    {
        val = 1;
    }

    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->write_control(m_v4l2_id, val);
        if (!ret)
        {
            return ret.as_failure();
        }
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}


V4L2PropertyCommandImpl::V4L2PropertyCommandImpl(struct v4l2_queryctrl* queryctrl,
                                                 struct v4l2_ext_control* /*ctrl*/,
                                                 std::shared_ptr<V4L2PropertyBackend> backend,
                                                 const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    m_name = (char*)queryctrl->name;

    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            m_name = mapping->gen_name;
        }
    }

    m_v4l2_id = queryctrl->id;

    m_cam = backend;
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
}


outcome::result<void> V4L2PropertyCommandImpl::execute()
{

    if (auto ptr = m_cam.lock())
    {
        auto ret = ptr->write_control(m_v4l2_id, 1);
        if (!ret)
        {
            return ret.as_failure();
        }
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}


V4L2PropertyEnumImpl::V4L2PropertyEnumImpl(struct v4l2_queryctrl* queryctrl,
                                           struct v4l2_ext_control* ctrl,
                                           std::shared_ptr<V4L2PropertyBackend> backend,
                                           const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    m_cam = backend;
    m_v4l2_id = queryctrl->id;

    if (!mapping)
    {
        m_name = (char*)queryctrl->name;

        if (auto ptr = m_cam.lock())
        {
            m_entries = ptr->get_menu_entries(m_v4l2_id, queryctrl->maximum);
        }
        else
        {
            SPDLOG_WARN("Unable to retrieve enum entries during property creation.");
        }
    }
    else
    {
        if (!mapping->gen_name.empty())
        {
            m_name = mapping->gen_name;
        }
        else
        {
            m_name = (char*)queryctrl->name;
        }

        //if (mapping->gen_enum_entries)
        if (mapping->conversion_type == tcam::v4l2::MappingType::IntToEnum)
        {
            auto res = tcam::v4l2::find_menu_entries(m_v4l2_id);
            if (res)
            {
                m_entries = res.value();
            }
            //mapping->gen_conversion.value().conversion.menu_entries;
        }
        else
        {
            if (auto ptr = m_cam.lock())
            {
                m_entries = ptr->get_menu_entries(m_v4l2_id, queryctrl->maximum);
            }
            else
            {
                SPDLOG_WARN("Unable to retrieve enum entries during property creation.");
            }
        }
    }
    m_flags = (PropertyFlags::Available | PropertyFlags::Implemented);
    m_default = m_entries.at(ctrl->value);
}


outcome::result<void> V4L2PropertyEnumImpl::valid_value(int value)
{
    auto it = m_entries.find(value);

    if (it == m_entries.end())
    {
        return tcam::status::PropertyValueDoesNotExist;
    }

    return outcome::success();
}


outcome::result<void> V4L2PropertyEnumImpl::set_value_str(const std::string& new_value)
{
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it)
    {
        if (it->second == new_value)
        {
            OUTCOME_TRY(set_value(it->first));
            return outcome::success();
        }
    }
    return tcam::status::PropertyDoesNotExist;
}


outcome::result<void> V4L2PropertyEnumImpl::set_value(int64_t new_value)
{
    if (!valid_value(new_value))
    {
        return tcam::status::PropertyValueDoesNotExist;
    }

    if (auto ptr = m_cam.lock())
    {
        OUTCOME_TRY(ptr->write_control(m_v4l2_id, new_value));
        return outcome::success();
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return tcam::status::ResourceNotLockable;
    }
}


outcome::result<std::string> V4L2PropertyEnumImpl::get_value() const
{
    OUTCOME_TRY(int value, get_value_int());

    // TODO: additional checks if key exists

    return m_entries.at(value);
}


outcome::result<int64_t> V4L2PropertyEnumImpl::get_value_int() const
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


std::vector<std::string> V4L2PropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    for (auto it = m_entries.begin(); it != m_entries.end(); ++it) { v.push_back(it->second); }
    return v;
}

} // namespace tcam::property
