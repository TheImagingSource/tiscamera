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

/*
  This file contains methods that are considered legacy
  but are still used when enabling a legacy mode.
 */


#include "PropertyGeneration.h"
#include "V4l2Device.h"
#include "logging.h"
#include "standard_properties.h"
#include "utils.h"
#include "v4l2_utils.h"
#include "v4l2_uvc_identifier.h"

#include <linux/videodev2.h>
#include <unistd.h> // usleep

using namespace tcam;


void V4l2Device::create_emulated_properties()
{
    auto tmp_props =
        generate_simulated_properties(property_handler->create_property_vector(), property_handler);

    for (auto& p : tmp_props)
    {
        property_description pd = { EMULATED_PROPERTY, 0, false, p };
        SPDLOG_DEBUG("Adding '{}' to property list", p->get_name().c_str());
        property_handler->properties.push_back(pd);
    }
}

void V4l2Device::sort_properties()
{
    if (property_handler->properties.empty())
    {
        return;
    }

    int id;
    auto search_func = [&id](const property_description& desc) {
        if (desc.id == id)
        {
            return true;
        }
        return false;
    };

    // ensure only one exposure interface is published
    id = TCAM_V4L2_EXPOSURE_TIME_US;
    auto exp_prop = std::find_if(
        property_handler->properties.begin(), property_handler->properties.end(), search_func);

    if (exp_prop != property_handler->properties.end())
    {
        // exposure time us is similar to the interface we want to publish
        // we therefor now search for all other exposure properties and move them
        // to the special section in case we need them

        auto p = property_handler->properties.begin();
        while (p != property_handler->properties.end())
        {
            if (p->id != TCAM_V4L2_EXPOSURE_TIME_US && p->prop->get_ID() == TCAM_PROPERTY_EXPOSURE)
            {
                property_handler->special_properties.push_back(*p);
                p = property_handler->properties.erase(p);
            }
            p++;
        }
    }

    // find properties that may exist multiple times
    // due to different interfaces
    std::vector<property_description> exp_desc;
    std::vector<property_description> trigger_desc;

    for (auto& prop_desc : property_handler->properties)
    {
        if (prop_desc.prop->get_ID() == TCAM_PROPERTY_EXPOSURE_AUTO)
        {
            exp_desc.push_back(prop_desc);
        }
        else if (prop_desc.prop->get_ID() == TCAM_PROPERTY_TRIGGER_MODE)
        {
            trigger_desc.push_back(prop_desc);
        }
    }

    if (exp_desc.size() > 1)
    {
        SPDLOG_INFO("Detected multiple exposure interfaces. Simplifying");
        // prefer exposure-auto over auto-shutter
        for (auto& p : exp_desc)
        {
            // if (p.id == 0x009a0901) // exposure-auto
            if (p.id == 0x0199e202) // auto-shutter
            {
                continue;
            }

            property_handler->special_properties.push_back(p);

            id = p.id;
            auto iter = std::find_if(property_handler->properties.begin(),
                                     property_handler->properties.end(),
                                     search_func);
            property_handler->properties.erase(iter);
        }
    }

    // ensure only one trigger
    // prefer the one without the uvc extensions

    if (trigger_desc.size() > 1)
    {
        SPDLOG_INFO("Detected multiple trigger interfaces. Simplifying");
        for (auto& p : trigger_desc)
        {
            if (p.id == V4L2_CID_PRIVACY) // prefer 'privacy'
            {
                continue;
            }

            property_handler->special_properties.push_back(p);

            id = p.id;
            auto iter = std::find_if(property_handler->properties.begin(),
                                     property_handler->properties.end(),
                                     search_func);
            property_handler->properties.erase(iter);
        }
    }
}

std::shared_ptr<Property> V4l2Device::apply_conversion_factor(std::shared_ptr<Property> prop,
                                                              const double factor)
{

    auto s = prop->get_struct();

    if (s.type == TCAM_PROPERTY_TYPE_INTEGER)
    {
        s.value.i.min *= factor;
        s.value.i.max *= factor;
        s.value.i.step *= factor;
        s.value.i.value *= factor;
        s.value.i.default_value *= factor;


        return std::make_shared<Property>(
            PropertyInteger(property_handler, s, prop->get_value_type()));
    }
    else if (s.type == TCAM_PROPERTY_TYPE_DOUBLE)
    {
        s.value.d.min *= factor;
        s.value.d.max *= factor;
        s.value.d.step *= factor;
        s.value.d.value *= factor;
        s.value.d.default_value *= factor;

        return std::make_shared<Property>(
            PropertyDouble(property_handler, s, prop->get_value_type()));
    }
    else
    {
        SPDLOG_ERROR(
            "Trying to apply conversion factor to property that does not represent numbers!");
        return nullptr;
    }
}

void V4l2Device::create_conversion_factors()
{
    if (property_handler->properties.empty())
    {
        return;
    }

    TCAM_PROPERTY_ID id;
    auto search_func = [&id](const property_description& desc) {
        if (desc.prop->get_ID() == id)
        {
            return true;
        }
        return false;
    };

    id = TCAM_PROPERTY_EXPOSURE;
    auto exposure = std::find_if(
        property_handler->properties.begin(), property_handler->properties.end(), search_func);

    if (exposure != property_handler->properties.end())
    {
        if (exposure->id == TCAM_V4L2_EXPOSURE_TIME_US)
        {
            // do nothing already the correct unit
            exposure->conversion_factor = 0.0;
        }
        else
        {
            // we have exposure_absolute which is in 100µs
            exposure->conversion_factor = 100.0;
            auto new_one = apply_conversion_factor(exposure->prop, exposure->conversion_factor);

            if (new_one != nullptr)
            {
                exposure->prop = new_one;
            }
        }
    }
}

void V4l2Device::index_all_controls(std::shared_ptr<PropertyImpl> impl)
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
        index_control(&qctrl, impl);
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }

    // sort out duplicated interfaces
    sort_properties();
    // create conversion factors so that properties allways use the same units
    create_conversion_factors();
    // create library only properties
    create_emulated_properties();
}


int V4l2Device::index_control(struct v4l2_queryctrl* qctrl, std::shared_ptr<PropertyImpl> impl)
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

    std::shared_ptr<Property> p;

    std::vector<int> special_properties = { 0x009a0901 /* exposure auto */ };

    if (std::find(special_properties.begin(), special_properties.end(), ext_ctrl.id)
        != special_properties.end())
    {
        create_special_property(fd, qctrl, &ext_ctrl, impl);
        return 0;
    }
    else
    {
        p = create_property(fd, qctrl, &ext_ctrl, property_handler);

        if (p == nullptr)
        {
            SPDLOG_ERROR("Property '{}' is null", qctrl->name);
            return -1;
        }
    }

    struct property_description desc;

    desc.id = qctrl->id;
    desc.conversion_factor = 0.0;
    desc.prop = p;

    static std::vector<TCAM_PROPERTY_ID> special_controls = {};

    if (std::find(special_controls.begin(), special_controls.end(), p->get_ID())
        != special_controls.end())
    {
        property_handler->special_properties.push_back(desc);
    }
    else
    {
        property_handler->properties.push_back(desc);
    }

    if (qctrl->type == V4L2_CTRL_TYPE_STRING)
    {
        free(ext_ctrl.string);
    }
    return 1;
}

void V4l2Device::create_special_property(int _fd,
                                         struct v4l2_queryctrl* queryctrl,
                                         struct v4l2_ext_control* ctrl,
                                         std::shared_ptr<PropertyImpl> impl)
{

    if (ctrl->id == 0x009a0901) // exposure auto map
    {
        auto prop_id = find_v4l2_mapping(ctrl->id);
        auto ctrl_m = get_control_reference(prop_id);
        uint32_t flags = convert_v4l2_flags(queryctrl->flags);

        tcam_device_property cp = {};


        //create internal property
        // cp.name can be empty as it is internal
        cp.id = generate_unique_property_id();
        cp.type = TCAM_PROPERTY_TYPE_ENUMERATION;
        cp.value.i.min = queryctrl->minimum;
        cp.value.i.max = queryctrl->maximum;
        cp.value.i.step = 0;
        cp.value.i.default_value = queryctrl->default_value;
        cp.value.i.value = ctrl->value;
        cp.flags = flags;

        struct v4l2_querymenu qmenu = {};

        qmenu.id = queryctrl->id;

        std::map<std::string, int> m;

        for (int i = 0; i <= queryctrl->maximum; i++)
        {
            qmenu.index = i;
            if (tcam_xioctl(_fd, VIDIOC_QUERYMENU, &qmenu))
                continue;

            std::string map_string((char*)qmenu.name);
            m.emplace(map_string, i);
        }

        auto internal_prop =
            std::make_shared<Property>(PropertyEnumeration(impl, cp, m, Property::ENUM));
        //handler->special_properties.push_back

        property_handler->special_properties.push_back(
            { (int)ctrl->id, 0.0, false, internal_prop });

        int active_value = cp.value.i.value;
        int default_value = cp.value.i.default_value;
        cp = {};

        /*
          1 => manual mode
          3 => aperture priority mode
          default is 3

          external bool is:
          true => 3
          false => 1

        */
        std::map<bool, std::string> mapping;
        mapping.emplace(true, "Aperture Priority Mode");
        mapping.emplace(false, "Manual Mode");

        cp = create_empty_property(ctrl_m.id);

        // create external property
        if (default_value == 1)
        {
            cp.value.b.default_value = false;
        }
        else if (default_value == 3)
        {
            cp.value.b.default_value = true;
        }
        else
        {
            SPDLOG_ERROR("Boolean '{}' has impossible default value: {} Setting to false",
                         cp.name,
                         queryctrl->default_value);
            cp.value.b.default_value = false;
        }

        if (active_value == 1)
        {
            cp.value.b.value = false;
        }
        else if (active_value == 3)
        {
            cp.value.b.value = true;
        }
        else
        {
            SPDLOG_ERROR("Boolean '{}' has impossible value: {} Setting to false",
                         cp.name,
                         (int)ctrl->value);
            cp.value.b.value = false;
        }
        cp.flags = flags;

        auto external_prop =
            std::make_shared<Property>(PropertyBoolean(impl, cp, Property::BOOLEAN));

        property_handler->properties.push_back({ (int)ctrl->id, 0.0, true, external_prop });

        property_handler->mappings.push_back({ external_prop, internal_prop, mapping });
    }
}

bool save_value_of_control(const v4l2_control* ctrl,
                           tcam_device_property* cp,
                           double conversion_factor)
{
    switch (cp->type)
    {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            if (ctrl->value == 0)
            {
                cp->value.b.value = false;
            }
            else if (ctrl->value > 0)
            {
                cp->value.b.value = true;
            }
            else
            {
                SPDLOG_ERROR("Boolean '{}' has impossible value: {} Setting to false",
                             cp->name,
                             ctrl->value);
                cp->value.b.value = false;
            }
            return true;
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            cp->value.i.value = ctrl->value;

            if (conversion_factor != 0.0)
            {
                cp->value.i.value *= conversion_factor;
            }
            return true;
        }
        default:
        {
            return false;
        }
    }
}

void V4l2Device::updateV4L2Property(V4l2Device::property_description& desc)
{
    struct v4l2_control ctrl = {};
    ctrl.id = desc.id;

    if (desc.prop->get_type() == TCAM_PROPERTY_TYPE_BUTTON)
    {
        return;
    }

    if (tcam_xioctl(fd, VIDIOC_G_CTRL, &ctrl))
    {
        SPDLOG_ERROR("Could not retrieve current value of {}. ioctl return '{}'",
                     desc.prop->get_name().c_str(),
                     strerror(errno));
    }

    auto cp = desc.prop->get_struct();

    save_value_of_control(&ctrl, &cp, desc.conversion_factor);
    SPDLOG_TRACE("Updated property {} to {}", desc.prop->get_name().c_str(), cp.value.i.value);

    desc.prop->set_struct(cp);
}


bool V4l2Device::changeV4L2Control(const property_description& prop_desc)
{

    TCAM_PROPERTY_TYPE type = prop_desc.prop->get_type();

    const std::string name = prop_desc.prop->get_name();

    if (name == "Exposure" || name == "ExposureTime" || name == "Exposure Time"
        || name == "Exposure Time (us)")
    {
        update_stream_timeout();
    }

    if (type == TCAM_PROPERTY_TYPE_STRING || type == TCAM_PROPERTY_TYPE_UNKNOWN
        || type == TCAM_PROPERTY_TYPE_DOUBLE)
    {
        SPDLOG_ERROR("Property type not supported. Property changes not submitted to device.");
        return false;
    }

    struct v4l2_control ctrl = {};

    ctrl.id = prop_desc.id;

    if (type == TCAM_PROPERTY_TYPE_INTEGER || type == TCAM_PROPERTY_TYPE_ENUMERATION)
    {
        ctrl.value = (std::static_pointer_cast<PropertyInteger>(prop_desc.prop))->get_value();
        if (prop_desc.conversion_factor != 0.0)
        {
            ctrl.value /= prop_desc.conversion_factor;
        }
    }
    else if (type == TCAM_PROPERTY_TYPE_BOOLEAN)
    {
        if ((std::static_pointer_cast<PropertyBoolean>(prop_desc.prop))->get_value())
        {
            ctrl.value = 1;
        }
        else
        {
            ctrl.value = 0;
        }
    }
    else if (type == TCAM_PROPERTY_TYPE_BUTTON)
    {
        ctrl.value = 1;
    }

    int ret = tcam_xioctl(fd, VIDIOC_S_CTRL, &ctrl);

    if (ret < 0)
    {
        SPDLOG_ERROR("Unable to submit property change for {}.",
                     prop_desc.prop->get_name().c_str());
    }
    else
    {
        SPDLOG_DEBUG(
            "Changed ctrl {} to value {}.", prop_desc.prop->get_name().c_str(), ctrl.value);
    }

    return true;
}
