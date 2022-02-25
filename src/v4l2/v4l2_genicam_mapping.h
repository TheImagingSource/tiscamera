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

#pragma once


#include "../PropertyInterfaces.h"
#include "V4L2PropertyBackend.h"
#include "v4l2_genicam_conversion.h"
#include "v4l2_utils.h"

#include <memory>
#include <string_view>

namespace tcam::v4l2
{

struct v4l2_genicam_mapping; // pre-declaration

enum class mapping_type
{
    normal,
    internal, // internal properties get added to the 'internal' property list
    blacklist, // blacklist properties will be ignored when building properties
};

struct v4l2_genicam_mapping_info
{
    uint32_t preferred_v4l2_id = 0;
    mapping_type mapping_type_ = mapping_type::normal;

    const v4l2_genicam_mapping* item = nullptr;
};

v4l2_genicam_mapping_info find_mapping_info(v4l2_device_type dev_type, uint32_t product_id, uint32_t v4l2_id);

auto create_mapped_prop(v4l2_device_type dev_type,
                        const std::vector<v4l2_queryctrl>& device_qctrl_list,
                        const v4l2_queryctrl& qctrl,
                        const v4l2_genicam_mapping& mapping,
                        const std::shared_ptr<tcam::v4l2::V4L2PropertyBackend>& p_property_backend)
    -> std::shared_ptr<tcam::property::IPropertyBase>;

std::vector<uint32_t> get_ordered_v4l2_id_list();

} /* namespace tcam::v4l2 */
