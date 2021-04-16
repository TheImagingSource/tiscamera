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

#include "v4l2_genicam_mapping.h"

#include "V4L2PropertyBackend.h"
#include "logging.h"

#include <memory>

namespace tcam::property
{

V4L2PropertyIntegerImpl::V4L2PropertyIntegerImpl(struct v4l2_queryctrl* queryctrl,
                                                 struct v4l2_ext_control* ctrl,
                                                 std::shared_ptr<V4L2PropertyBackend> backend,
                                                 const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    p_min = queryctrl->minimum;
    p_max = queryctrl->maximum;
    p_step = queryctrl->step;

    p_default = ctrl->value;

    p_name = (char*)queryctrl->name;
    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            p_name = mapping->gen_name;
        }
    }

    p_v4l2_id = queryctrl->id;
    p_cam = backend;
}


int64_t V4L2PropertyIntegerImpl::get_value() const
{
    int64_t value = 0;

    if (auto ptr = p_cam.lock())
    {
        ptr->read_control(p_v4l2_id, value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return -1;
    }

    return value;
}


bool V4L2PropertyIntegerImpl::set_value(int64_t new_value)
{
    if (!valid_value(new_value))
    {
        return false;
    }

    if (auto ptr = p_cam.lock())
    {
        ptr->write_control(p_v4l2_id, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return false;
    }

    return false;
}


bool V4L2PropertyIntegerImpl::valid_value(int64_t val)
{
    if (val % get_step() != 0)
    {
        return false;
    }

    if (get_min() > val || val > get_max())
    {
        return false;
    }

    return true;
}







V4L2PropertyDoubleImpl::V4L2PropertyDoubleImpl(struct v4l2_queryctrl* queryctrl,
                                               struct v4l2_ext_control* ctrl,
                                               std::shared_ptr<V4L2PropertyBackend> backend,
                                               const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    p_min = queryctrl->minimum;
    p_max = queryctrl->maximum;
    p_step = queryctrl->step;
    p_default = ctrl->value;

    p_name = (char*)queryctrl->name;

    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            p_name = mapping->gen_name;
        }
    }

    p_v4l2_id = queryctrl->id;

    p_cam = backend;
}


double V4L2PropertyDoubleImpl::get_value() const
{
    int64_t value = 0;

    if (auto ptr = p_cam.lock())
    {
        ptr->read_control(p_v4l2_id, value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return -1;
    }

    return value;
}


bool V4L2PropertyDoubleImpl::set_value(double new_value)
{
    if (!valid_value(new_value))
    {
        return false;
    }

    if (auto ptr = p_cam.lock())
    {
        ptr->write_control(p_v4l2_id, new_value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return false;
    }

    return false;
}



bool V4L2PropertyDoubleImpl::valid_value(double val)
{
    if (get_min() > val || val > get_max())
    {
        return false;
    }

    return true;
}





V4L2PropertyBoolImpl::V4L2PropertyBoolImpl(struct v4l2_queryctrl* queryctrl,
                                           struct v4l2_ext_control* ctrl,
                                           std::shared_ptr<V4L2PropertyBackend> backend,
                                           const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    if (ctrl->value == 0)
    {
        p_default = false;
    }
    else
    {
        p_default = true;
    }
    p_name = (char*)queryctrl->name;
    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            p_name = mapping->gen_name;
        }
    }

    p_v4l2_id = queryctrl->id;

    p_cam = backend;

}


bool V4L2PropertyBoolImpl::get_value() const
{
    int64_t value = 0;

    if (auto ptr = p_cam.lock())
    {
        ptr->read_control(p_v4l2_id, value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return false;
    }

    if (value == 0)
    {
        return false;
    }
    return true;
}

bool V4L2PropertyBoolImpl::set_value(bool new_value)
{
    int64_t val = 0;
    if (new_value)
    {
        val = 1;
    }

    if (auto ptr = p_cam.lock())
    {
        ptr->write_control(p_v4l2_id, val);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return false;
    }
    return true;
}





V4L2PropertyCommandImpl::V4L2PropertyCommandImpl(struct v4l2_queryctrl* queryctrl,
                                                 struct v4l2_ext_control* /*ctrl*/,
                                                 std::shared_ptr<V4L2PropertyBackend> backend,
                                                 const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    p_name = (char*)queryctrl->name;

    if (mapping)
    {
        if (!mapping->gen_name.empty())
        {
            p_name = mapping->gen_name;
        }
    }

    p_v4l2_id = queryctrl->id;

    p_cam = backend;
}


bool V4L2PropertyCommandImpl::execute()
{

    if (auto ptr = p_cam.lock())
    {
        ptr->write_control(p_v4l2_id, 1);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return false;
    }
    return true;}





V4L2PropertyEnumImpl::V4L2PropertyEnumImpl(struct v4l2_queryctrl* queryctrl,
                                           struct v4l2_ext_control* ctrl,
                                           std::shared_ptr<V4L2PropertyBackend> backend,
                                           const tcam::v4l2::v4l2_genicam_mapping* mapping)
{
    p_cam = backend;
    p_v4l2_id = queryctrl->id;

    if (!mapping)
    {
        p_name = (char*)queryctrl->name;

        if (auto ptr = p_cam.lock())
        {
            p_entries = ptr->get_menu_entries(p_v4l2_id, queryctrl->maximum);
        }
        else
        {
            SPDLOG_WARN("Unable to retrieve enum entries during proerty creation.");
        }
    }
    else
    {
        if (!mapping->gen_name.empty())
        {
            p_name = mapping->gen_name;
        }

        if (queryctrl->type == V4L2_CTRL_TYPE_BOOLEAN)
        {
            p_entries = {
                {0, "Off"},
                {1, "Continuous"}
            };
        }
    }

    p_default = p_entries.at(ctrl->value);
}


bool V4L2PropertyEnumImpl::valid_value(int value)
{
    auto it = p_entries.find(value);

    if (it == p_entries.end())
    {
        return false;
    }

    return true;
}


bool V4L2PropertyEnumImpl::set_value_str(const std::string& new_value)
{
    for (auto it = p_entries.begin(); it != p_entries.end(); ++it)
    {
        if (it->second == new_value)
        {
            return set_value(it->first);
        }
    }
    return false;
}


bool V4L2PropertyEnumImpl::set_value(int new_value)
{
    if (!valid_value(new_value))
    {
        return false;
    }

    if (auto ptr = p_cam.lock())
    {
        if (ptr->write_control(p_v4l2_id, new_value) != 0)
        {
            SPDLOG_ERROR("Something went wrong while writing {}", p_name);
        }
        else
        {
            //SPDLOG_DEBUG("Wrote {} {}", p_name, new_value);
        }
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot write value.");
        return false;
    }

    return false;
}


std::string V4L2PropertyEnumImpl::get_value() const
{
    int value = get_value_int();

    // TODO: additional checks if key exists

    return p_entries.at(value);
}


int V4L2PropertyEnumImpl::get_value_int() const
{
    int64_t value = 0;

    if (auto ptr = p_cam.lock())
    {
        ptr->read_control(p_v4l2_id, value);
    }
    else
    {
        SPDLOG_ERROR("Unable to lock v4l2 device backend. Cannot retrieve value.");
        return -1;
    }

    return value;
}


std::vector<std::string> V4L2PropertyEnumImpl::get_entries() const
{
    std::vector<std::string> v;
    for (auto it = p_entries.begin(); it != p_entries.end(); ++it)
    {
        v.push_back(it->second);
    }
    return v;
}

} // namespace tcam::property
