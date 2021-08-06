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

#include "../logging.h"
#include "../utils.h"
#include "V4l2Device.h"
#include "v4l2_genicam_mapping.h"
#include "v4l2_property_impl.h"
#include "v4l2_utils.h"

#include <linux/videodev2.h>
#include <unistd.h> // pipe, usleep


using namespace tcam;
using namespace tcam::v4l2;


std::shared_ptr<tcam::property::IPropertyBase> V4l2Device::new_control(struct v4l2_queryctrl* qctrl)
{
    if (qctrl->flags & V4L2_CTRL_FLAG_DISABLED)
    {
        return nullptr;
    }

    if (qctrl->type == V4L2_CTRL_TYPE_CTRL_CLASS)
    {
        // ignore unneccesary controls descriptions such as control "groups"
        return nullptr;
    }
    struct v4l2_ext_control ext_ctrl = {};
    struct v4l2_ext_controls ctrls = {};

    ext_ctrl.id = qctrl->id;
    ctrls.ctrl_class = V4L2_CTRL_ID2CLASS(qctrl->id);
    ctrls.count = 1;
    ctrls.controls = &ext_ctrl;

    if (V4L2_CTRL_ID2CLASS(qctrl->id) != V4L2_CTRL_CLASS_USER && qctrl->id < V4L2_CID_PRIVATE_BASE)
    {
        if (qctrl->type == V4L2_CTRL_TYPE_STRING)
        {
            ext_ctrl.size = qctrl->maximum + 1;
            ext_ctrl.string = (char*)malloc(ext_ctrl.size);
            ext_ctrl.string[0] = 0;
        }

        if (qctrl->flags & V4L2_CTRL_FLAG_WRITE_ONLY)
        {
            SPDLOG_INFO("Encountered write only control.");
        }

        else if (tcam_xioctl(m_fd, VIDIOC_G_EXT_CTRLS, &ctrls))
        {
            SPDLOG_ERROR("Errno {} getting ext_ctrl {}", errno, qctrl->name);
            return nullptr;
        }
    }
    else
    {
        struct v4l2_control ctrl = {};

        ctrl.id = qctrl->id;
        if (tcam_xioctl(m_fd, VIDIOC_G_CTRL, &ctrl))
        {
            SPDLOG_ERROR("error {} getting ctrl {}", errno, qctrl->name);
            return nullptr;
        }
        ext_ctrl.value = ctrl.value;
    }

    const struct v4l2_genicam_mapping* mapping = find_mapping(qctrl->id);

    // decide what property type to use through TCAM_PROPERTY_TYPE and not v4l2 property type
    // this way we can generate types v4l2 does not have, i.e. double
    TCAM_PROPERTY_TYPE type = v4l2_property_type_to_tcam(qctrl->type);

    if (mapping)
    {
        //SPDLOG_DEBUG("Conversion requried for qtrcl->name {}", qctrl->name);

        if (mapping->gen_type != TCAM_PROPERTY_TYPE_UNKNOWN)
        {
            type = mapping->gen_type;
        }
    }

    switch (type)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            return std::make_shared<tcam::property::V4L2PropertyIntegerImpl>(
                qctrl, &ext_ctrl, p_property_backend, mapping);
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            return std::make_shared<tcam::property::V4L2PropertyDoubleImpl>(
                qctrl, &ext_ctrl, p_property_backend, mapping);
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            return std::make_shared<tcam::property::V4L2PropertyEnumImpl>(
                qctrl, &ext_ctrl, p_property_backend, mapping);
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            return std::make_shared<tcam::property::V4L2PropertyCommandImpl>(
                qctrl, &ext_ctrl, p_property_backend, mapping);
        }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            return std::make_shared<tcam::property::V4L2PropertyBoolImpl>(
                qctrl, &ext_ctrl, p_property_backend, mapping);
        }
        case TCAM_PROPERTY_TYPE_STRING:
        default:
        {
            SPDLOG_WARN("Unknown v4l2 property type - %d.", qctrl->type);
            return nullptr;
        }
    }

    return 0;
}


void V4l2Device::sort_properties(
    std::map<uint32_t, std::shared_ptr<tcam::property::IPropertyBase>> properties)
{
    if (properties.empty())
    {
        return;
    }

    auto preserve_id = [&properties](uint32_t v4l2_id, const std::string& name) {
        int count = 0;
        for (const auto p : properties)
        {
            if (p.second->get_name() == name)
            {
                count++;
            }
        }

        // this means there is only one interface provider
        // nothing to remove
        if (count <= 1)
        {
            return;
        }

        auto p = properties.begin();
        while (p != properties.end())
        {
            if (p->first != v4l2_id && (p->second->get_name() == name))
            {
                SPDLOG_DEBUG("Found additional interface for {}({:x}). Hiding", name, v4l2_id);
                p = properties.erase(p);
            }
            p++;
        }
    };

    // the id is the property id we prefer using
    // for the named property
    preserve_id(0x199e201, "ExposureTime");
    preserve_id(0x0199e202, "ExposureTimeAuto");
    preserve_id(V4L2_CID_PRIVACY, "TriggerMode");
    preserve_id(0x199e204, "Gain");

    // do not drop certain properties
    // instead store them in a different container
    // for internal usage
    // things like binning, skipping, override scanning mode
    // are needed for later usage
    auto hide_properties = [this, &properties] (const std::string& name)
    {
        auto p = properties.begin();
        while(p != properties.end())
        {
            if (name == p->second->get_name())
            {
                m_internal_properties.push_back(p->second);
                properties.erase(p);
                return;
            }
            p++;
        }
    };

    hide_properties("Skipping");
    hide_properties("Binning");
    hide_properties("BinningHorizontal");
    hide_properties("BinningVertical");
    hide_properties("OverrideScanningMode");
    hide_properties("Scanning Mode Selector");
    hide_properties("Scanning Mode Identifier");
    hide_properties("Scanning Mode Binning Horizontal");
    hide_properties("Scanning Mode Binning Horizonta");
    hide_properties("Scanning Mode Binning Vertical");
    hide_properties("Scanning Mode Skipping Horizont");
    hide_properties("Scanning Mode Skipping Vertical");
    hide_properties("Scanning Mode Flags");

    m_properties.reserve(properties.size());

    for (auto it = properties.begin(); it != properties.end(); ++it)
    {
        m_properties.push_back(it->second);
    }
}


void V4l2Device::index_controls()
{
    bool extension_unit_exists = false;
    // test for loaded extension unit.
    for (unsigned int i = 0; i < 3; ++i)
    {
        if (extension_unit_is_loaded())
        {
            extension_unit_exists = true;
            break;
        }
        else
        {
            usleep(500);
        }
    }
    if (!extension_unit_exists)
    {
        SPDLOG_WARN(
            "The property extension unit does not exist. Not all properties will be accessible.");
    }

    std::map<uint32_t, std::shared_ptr<tcam::property::IPropertyBase>> new_properties;

    struct v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

    while (tcam_xioctl(this->m_fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        auto prop = new_control(&qctrl);

        if (prop)
        {
            new_properties.emplace(qctrl.id, prop);
            //sort_properties(qctrl.id, prop);
        }

        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }

    sort_properties(new_properties);
}
