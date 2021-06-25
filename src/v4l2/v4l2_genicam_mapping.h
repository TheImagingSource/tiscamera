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


#include "base_types.h"
#include "v4l2_genicam_conversion.h"

#include <map>
#include <optional>
#include <string>

namespace tcam::v4l2
{

struct v4l2_genicam_mapping
{
    uint32_t v4l2_id;
    std::string gen_name; // empty if name can be kept

    // type that shall be used, TCAM_PROPERTY_TYPE_UNKNOWN if type can stay
    TCAM_PROPERTY_TYPE gen_type;
    MappingType conversion_type; // information concerning additional conversion steps
};


// check if mapping exists for v4l2 property id
// returns mapping description if existent
// nullptr otherwise
// ownership is not given
const v4l2_genicam_mapping* find_mapping(uint32_t v4l2_id);

} /* namespace tcam::v4l2 */
