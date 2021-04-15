
#pragma once


#include <string>
#include "base_types.h"

namespace tcam::v4l2
{

struct v4l2_genicam_mapping
{
    std::string gen_name; // empty if name can be kept
    TCAM_PROPERTY_TYPE gen_type; // type that shall be used, TCAM_PROPERTY_TYPE_UNKNOWN if type can stay
};


// check if mapping exists for v4l2 property id
// returns mapping description if existent
// nullptr otherwise
// ownership is not given
const v4l2_genicam_mapping* find_mapping(uint32_t v4l2_id);

} /* namespace tcam::v4l2 */
