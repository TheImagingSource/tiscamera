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

#include <linux/videodev2.h>
#include <tcamprop1.0_base/tcamprop_property_info_list.h>


using namespace tcam::v4l2;

static auto create_unmapped_prop(
    const v4l2_queryctrl& qctrl,
    const std::shared_ptr<tcam::v4l2::V4L2PropertyBackend>& p_property_backend)
    -> std::shared_ptr<tcam::property::IPropertyBase>
{
    using namespace tcam::v4l2;

    switch (qctrl.type)
    {
        case V4L2_CTRL_TYPE_BOOLEAN:
            return std::make_shared<V4L2PropertyBoolImpl>(qctrl, p_property_backend);
        case V4L2_CTRL_TYPE_INTEGER:
            return std::make_shared<V4L2PropertyIntegerImpl>(qctrl, p_property_backend);
        case V4L2_CTRL_TYPE_INTEGER_MENU:
        case V4L2_CTRL_TYPE_MENU:
            return std::make_shared<V4L2PropertyEnumImpl>(qctrl, p_property_backend);
        case V4L2_CTRL_TYPE_BUTTON:
            return std::make_shared<V4L2PropertyCommandImpl>(qctrl, p_property_backend);
        case V4L2_CTRL_TYPE_STRING:
        default:
        {
            SPDLOG_WARN("Unknown v4l2 property type - {}.", qctrl.type);
            return nullptr;
        }
    }
    return nullptr;
}

void tcam::V4l2Device::generate_properties(const std::vector<v4l2_queryctrl>& qctrl_list)
{
    const auto dev_type = v4l2::get_device_type(this->device);
    const auto product_id = v4l2::fetch_product_id(this->device);

    m_properties.reserve(qctrl_list.size());
    m_internal_properties.reserve(qctrl_list.size());

    auto ordered_id_list = tcam::v4l2::get_ordered_v4l2_id_list();

    for (auto v4l2_id_to_test : ordered_id_list)
    {
        auto f = std::find_if(qctrl_list.begin(),
                              qctrl_list.end(),
                              [v4l2_id_to_test](auto& ctrl) { return ctrl.id == v4l2_id_to_test; });
        if (f == qctrl_list.end()) {
            continue;
        }

        const auto& qctrl = *f;

        auto map_info = tcam::v4l2::find_mapping_info(dev_type, product_id, qctrl.id);
        if (map_info.preferred_v4l2_id && is_id_present(qctrl_list, map_info.preferred_v4l2_id))
        {
            SPDLOG_TRACE("Skipping property id={:#x} due to presence of id={:#x}.",
                         qctrl.id,
                         map_info.preferred_v4l2_id);
            continue;
        }
        if (map_info.mapping_type_ == mapping_type::blacklist)
        {
            SPDLOG_TRACE("Skipping property id={:#x}, because it is blacklisted.", qctrl.id);
            continue;
        }

        std::shared_ptr<tcam::property::IPropertyBase> prop_ptr;
        if (!map_info.item)
        {
            if (map_info.mapping_type_
                != mapping_type::internal) // there was an entry but no mapping
            {
                SPDLOG_WARN("Failed to find mapping entry for v4l2 ctrl id=0x{:x}, name='{}'.",
                            qctrl.id,
                            (char*)qctrl.name);
            }
            prop_ptr = create_unmapped_prop(qctrl, p_property_backend);
        }
        else
        {
            prop_ptr =
                create_mapped_prop(dev_type, qctrl_list, qctrl, *map_info.item, p_property_backend);
        }

        if (prop_ptr)
        {
            if (map_info.mapping_type_ == mapping_type::internal)
            {
                m_internal_properties.push_back(prop_ptr);
            }
            else
            {
                m_properties.push_back( prop_ptr );
            }
        }
    }
}

void tcam::V4l2Device::update_dependency_information()
{
    //
    // v4l2/uvc devices do not have proper interdependencies for properties
    // manually reapply some of them
    // this causes things like ExposureAuto==On to lock ExposureTime
    //
    for (auto& prop : m_properties)
    {
        auto prop_lock_ptr = std::dynamic_pointer_cast<tcam::property::PropertyLock>(prop);
        if (!prop_lock_ptr)
            continue;

        auto dependent_names = prop_lock_ptr->get_dependent_names();
        if (dependent_names.empty())
            continue;

        std::vector<std::weak_ptr<tcam::property::PropertyLock>> dependencies_to_include;
        for (const auto& name : dependent_names)
        {
            auto dependency = find_property(m_properties, name);
            if (dependency)
            {
                auto ptr = std::dynamic_pointer_cast<tcam::property::PropertyLock>(dependency);
                if (ptr)
                {
                    dependencies_to_include.push_back(ptr);
                }
            }
        }
        if (!dependencies_to_include.empty())
        {
            prop_lock_ptr->set_dependent_properties(std::move(dependencies_to_include));
        }
    }
}

bool tcam::V4l2Device::load_extension_unit()
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


void tcam::V4l2Device::create_properties()
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

    while (tcam::tcam_xioctl(this->m_fd, VIDIOC_QUERYCTRL, &qctrl) == 0)
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

void tcam::V4l2Device::create_videoformat_dependent_properties()
{
    auto dim = get_sensor_size();
    if( find_property( m_properties, tcamprop1::prop_list::SensorWidth.name ) == nullptr )
    {
        auto prop_dim_width = std::make_shared<tcam::v4l2::prop_impl_sensor_dim>(
            &tcamprop1::prop_list::SensorWidth, dim.width );
        auto prop_dim_height = std::make_shared<tcam::v4l2::prop_impl_sensor_dim>(
            &tcamprop1::prop_list::SensorHeight, dim.height );

        m_properties.push_back( prop_dim_width );
        m_properties.push_back( prop_dim_height );
    }

    if( auto auto_center = tcam::v4l2::prop_impl_offset_auto_center::create_if_needed( m_properties, dim ); auto_center )
    {
        m_properties.push_back( auto_center );
        software_auto_center_ = auto_center;
    }

    update_dependency_information();
}

void    tcam::V4l2Device::update_properties( const VideoFormat& current_fmt )
{
    if( software_auto_center_ )
    {
        software_auto_center_->set_format( current_fmt );
    }
}