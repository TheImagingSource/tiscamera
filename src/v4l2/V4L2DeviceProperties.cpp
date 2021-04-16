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

#include "V4l2Device.h"

#include "v4l2_utils.h"
#include "v4l2_property_impl.h"
#include "v4l2_genicam_mapping.h"

#include "logging.h"
#include "utils.h"

#include <linux/videodev2.h>
#include <unistd.h> // pipe, usleep


using namespace tcam;
using namespace tcam::v4l2;


int V4l2Device::new_control(struct v4l2_queryctrl* qctrl)
{
    if (qctrl->flags & V4L2_CTRL_FLAG_DISABLED)
    {
        return 1;
    }

    if (qctrl->type == V4L2_CTRL_TYPE_CTRL_CLASS)
    {
        // ignore unneccesary controls descriptions such as control "groups"
        return 1;
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

        else if (tcam_xioctl(fd, VIDIOC_G_EXT_CTRLS, &ctrls))
        {
            SPDLOG_ERROR("Errno {} getting ext_ctrl {}", errno, qctrl->name);
            return -1;
        }
    }
    else
    {
        struct v4l2_control ctrl = {};

        ctrl.id = qctrl->id;
        if (tcam_xioctl(fd, VIDIOC_G_CTRL, &ctrl))
        {
            SPDLOG_ERROR("error {} getting ctrl {}", errno, qctrl->name);
            return -1;
        }
        ext_ctrl.value = ctrl.value;
    }

    const struct v4l2_genicam_mapping* mapping = find_mapping(qctrl->id);

    // decide what property type to use through TCAM_PROPERTY_TYPE and not v4l2 property type
    // this way we can generate types v4l2 does not have, i.e. double
    TCAM_PROPERTY_TYPE type = v4l2_property_type_to_tcam(qctrl->type);

    if (mapping)
    {
        SPDLOG_DEBUG("Conversion requried for qtrcl->name {}", qctrl->name);

        type = mapping->gen_type;
    }

    switch (type)
    {
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            p_properties.push_back(std::make_shared<tcam::property::V4L2PropertyIntegerImpl>(qctrl, &ext_ctrl, p_property_backend, mapping));
            break;
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            p_properties.push_back(std::make_shared<tcam::property::V4L2PropertyDoubleImpl>(qctrl, &ext_ctrl, p_property_backend, mapping));
            break;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            p_properties.push_back(std::make_shared<tcam::property::V4L2PropertyEnumImpl>(qctrl, &ext_ctrl, p_property_backend, mapping));
            break;
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            p_properties.push_back(std::make_shared<tcam::property::V4L2PropertyCommandImpl>(qctrl, &ext_ctrl, p_property_backend, mapping));
            break;
        }
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            p_properties.push_back(std::make_shared<tcam::property::V4L2PropertyBoolImpl>(qctrl, &ext_ctrl, p_property_backend, mapping));
            break;
        }
        case TCAM_PROPERTY_TYPE_STRING:
        default:
        {
            SPDLOG_WARN("Unknown v4l2 property type - %d.", qctrl->type);
            return 1;
        }
    }

    return 0;
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


    struct v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

    while (tcam_xioctl(this->fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        new_control(&qctrl);
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
}
