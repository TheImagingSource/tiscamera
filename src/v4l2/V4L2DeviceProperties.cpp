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
#include "../property_dependencies.h"
#include "../utils.h"
#include "V4l2Device.h"
#include "uvc-extension-loader.h"
#include "v4l2_genicam_mapping.h"
#include "v4l2_property_impl.h"
#include "v4l2_utils.h"

#include <algorithm>
#include <linux/videodev2.h>
#include <unistd.h> // pipe, usleep


using namespace tcam;
using namespace tcam::v4l2;

static auto create_mapped_prop(
    const v4l2_queryctrl& qctrl,
    const v4l2_genicam_mapping& mapping,
    const std::shared_ptr<tcam::property::V4L2PropertyBackend>& p_property_backend)
    -> std::shared_ptr<tcam::property::IPropertyBase>
{
    using namespace tcam::property;

    switch (mapping.info_property_type_)
    {
        case tcamprop1::prop_type::Boolean:
        {
            return std::make_shared<V4L2PropertyBoolImpl>(
                qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_boolean*>(mapping.info_));
        }
        case tcamprop1::prop_type::Integer:
        {
            return std::make_shared<V4L2PropertyIntegerImpl>(
                qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_integer*>(mapping.info_),
                mapping.converter_);
        }
        case tcamprop1::prop_type::Float:
        {
            return std::make_shared<V4L2PropertyDoubleImpl>(
                qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_float*>(mapping.info_),
                mapping.converter_);
        }
        case tcamprop1::prop_type::Command:
        {
            return std::make_shared<V4L2PropertyCommandImpl>(
                qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_command*>(mapping.info_));
        }
        case tcamprop1::prop_type::Enumeration:
        {
            return std::make_shared<V4L2PropertyEnumImpl>(
                qctrl,
                p_property_backend,
                static_cast<const tcamprop1::prop_static_info_enumeration*>(mapping.info_),
                mapping.fetch_menu_entries_);
        }
    }
    return nullptr;
}

static auto create_unmapped_prop(
    const v4l2_queryctrl& qctrl,
    const std::shared_ptr<tcam::property::V4L2PropertyBackend>& p_property_backend)
    -> std::shared_ptr<tcam::property::IPropertyBase>
{
    using namespace tcam::property;

    TCAM_PROPERTY_TYPE type = v4l2_property_type_to_tcam(qctrl.type);
    switch (type)
    {
        case TCAM_PROPERTY_TYPE_BOOLEAN:
        {
            return std::make_shared<V4L2PropertyBoolImpl>(qctrl, p_property_backend);
        }
        case TCAM_PROPERTY_TYPE_INTEGER:
        {
            return std::make_shared<V4L2PropertyIntegerImpl>(qctrl, p_property_backend);
        }
        case TCAM_PROPERTY_TYPE_DOUBLE:
        {
            return std::make_shared<V4L2PropertyDoubleImpl>(qctrl, p_property_backend);
        }
        case TCAM_PROPERTY_TYPE_ENUMERATION:
        {
            return std::make_shared<V4L2PropertyEnumImpl>(qctrl, p_property_backend);
        }
        case TCAM_PROPERTY_TYPE_BUTTON:
        {
            return std::make_shared<V4L2PropertyCommandImpl>(qctrl, p_property_backend);
        }
        case TCAM_PROPERTY_TYPE_STRING:
        case TCAM_PROPERTY_TYPE_UNKNOWN:
        {
            SPDLOG_WARN("Unknown v4l2 property type - {}.", qctrl.type);
            return nullptr;
        }
    }
    return nullptr;
}

void V4l2Device::generate_properties(const std::vector<v4l2_queryctrl>& qctrl_list)
{
    auto is_id_present = [&qctrl_list](uint32_t id_to_look_for)
    {
        return std::any_of(qctrl_list.begin(),
                           qctrl_list.end(),
                           [id_to_look_for](const v4l2_queryctrl& ctrl)
                           { return ctrl.id == id_to_look_for; });
    };

    for (const auto& qctrl : qctrl_list)
    {
        std::shared_ptr<tcam::property::IPropertyBase> prop_ptr;

        bool is_internal_property = false;
        const v4l2_genicam_mapping* mapping = find_mapping(qctrl.id);
        if (!mapping)
        {
            SPDLOG_WARN("Failed to find mapping entry for v4l2 ctrl id=0x{:x}, name='{}'.",
                        qctrl.id,
                        (char*)qctrl.name);

            prop_ptr = create_unmapped_prop(qctrl, p_property_backend);
        }
        else
        {
            if (mapping->preferred_id_ && is_id_present(mapping->preferred_id_))
            {
                SPDLOG_TRACE("Skipping property id={:#x} due to presence of id={:#x}.",
                             qctrl.id,
                             mapping->preferred_id_);
                continue;
            }

            is_internal_property = mapping->mapping_type_ == mapping_type::internal;

            if (mapping->info_)
            {
                prop_ptr = create_mapped_prop(qctrl, *mapping, p_property_backend);
            }
            else
            {
                prop_ptr = create_unmapped_prop(qctrl, p_property_backend);
            }
        }

        if (prop_ptr)
        {
            if (is_internal_property)
            {
                m_internal_properties.push_back(prop_ptr);
            }
            else
            {
                m_properties.push_back(prop_ptr);
            }
        }
    }

    update_dependency_information();
}

void V4l2Device::update_dependency_information()
{
    //
    // v4l2/uvc devices do not have proper interdependencies for properties
    // manually reapply some of them
    // this causes things like ExposureAuto==On to lock ExposureTime
    //
    for (auto& prop : m_properties)
    {
        std::vector<std::weak_ptr<tcam::property::PropertyLock>> dependencies_to_include;
        auto dependencies = tcam::property::find_dependency(prop->get_name());

        if (dependencies)
        {
            for (const auto& dep : dependencies->dependencies)
            {
                auto itere = std::find_if(m_properties.begin(),
                                          m_properties.end(),
                                          [&dep](const auto& p) { return p->get_name() == dep; });

                if (itere != m_properties.end())
                {
                    auto ptr = std::dynamic_pointer_cast<tcam::property::PropertyLock>(*itere);
                    dependencies_to_include.push_back(ptr);
                }
            }
        }
        if (!dependencies_to_include.empty())
        {
            auto ptr = std::dynamic_pointer_cast<tcam::property::PropertyLock>(prop);
            if (ptr)
            {
                ptr->set_dependencies(dependencies_to_include);
            }
        }
    }
}

bool V4l2Device::load_extension_unit()
{
    auto message_cb = [](const std::string& message)
    {
        SPDLOG_DEBUG("{}", message.c_str());
    };

    std::string extension_file =
        tcam::uvc::determine_extension_file(this->device.get_info().additional_identifier);

    if (extension_file.empty())
    {
        SPDLOG_WARN("Unable to determine uvc extension file");
        return false;
    }

    auto mappings = tcam::uvc::load_description_file(extension_file, message_cb);
    if (mappings.empty())
    {
        SPDLOG_WARN("Unable to load uvc extension file");
        return false;
    }
    tcam::uvc::apply_mappings(m_fd, mappings, message_cb);

    return true;
}


void V4l2Device::index_controls()
{
    bool extension_unit_exists = extension_unit_is_loaded();

    if (!extension_unit_exists)
    {
        extension_unit_exists = load_extension_unit();
    }

    if (!extension_unit_exists)
    {
        SPDLOG_WARN(
            "The property extension unit does not exist. Not all properties will be accessible.");
    }

    v4l2_queryctrl qctrl = {};
    qctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;

    std::vector<v4l2_queryctrl> qctrl_av;

    while (tcam_xioctl(this->m_fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
    {
        // ignore unnecessary control descriptions such as control "groups"
        if (!(qctrl.flags & V4L2_CTRL_FLAG_DISABLED) && qctrl.type != V4L2_CTRL_TYPE_CTRL_CLASS)
        {
            qctrl_av.push_back(qctrl);
        }
        qctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }

    generate_properties(qctrl_av);
}
