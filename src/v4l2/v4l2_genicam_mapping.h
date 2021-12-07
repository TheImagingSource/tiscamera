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


#include "../base_types.h"
#include "v4l2_genicam_conversion.h"

#include <string_view>
#include <tcamprop1.0_base/tcamprop_base.h>
#include <tcamprop1.0_base/tcamprop_property_info.h>

namespace tcam::v4l2
{

enum class mapping_type {
    normal,
    internal,
};

struct v4l2_genicam_mapping
{
    constexpr v4l2_genicam_mapping(
        uint32_t id,
        mapping_type type) noexcept
        : v4l2_id(id), mapping_type_( type )
    {
    }

    template<class Tprop_static_info>
    constexpr v4l2_genicam_mapping(uint32_t id, const Tprop_static_info* info_type, uint32_t preferred_id = 0)
        : v4l2_id(id), preferred_id_( preferred_id ), info_(info_type), info_property_type_(Tprop_static_info::property_type)
    {
    }
    template<class Tprop_static_info>
    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const Tprop_static_info* info_type,
                                   converter_scale converter, uint32_t preferred_id = 0 )
        : v4l2_id(id), preferred_id_( preferred_id ), info_(info_type),
          info_property_type_(Tprop_static_info::property_type), converter_ { converter }
    {
    }

    constexpr v4l2_genicam_mapping(uint32_t id,
                                   const tcamprop1::prop_static_info_enumeration* info_type,
                                   fetch_menu_entries_func func, uint32_t preferred_id = 0 )
        : v4l2_id(id), preferred_id_( preferred_id ), info_(info_type), info_property_type_(tcamprop1::prop_type::Enumeration),
          fetch_menu_entries_(func)
    {
    }

    uint32_t v4l2_id = 0;   // only used for lookup
    uint32_t preferred_id_ = 0; // when a control with this id is present, use that instead of this one

    tcamprop1::prop_static_info const* info_ = nullptr;
    tcamprop1::prop_type info_property_type_ = tcamprop1::prop_type::Boolean;

    tcam::v4l2::converter_scale converter_ = {};                // only valid for tcamprop1::prop_type::Integer and tcamprop1::prop_type::Float
    fetch_menu_entries_func fetch_menu_entries_ = nullptr;      // only valid for tcamprop1::prop_type::Enumeration

    mapping_type mapping_type_ = mapping_type::normal;
};


// check if mapping exists for v4l2 property id
// returns mapping description if existent
// nullptr otherwise
// ownership is not given
const v4l2_genicam_mapping* find_mapping(uint32_t v4l2_id);

} /* namespace tcam::v4l2 */
