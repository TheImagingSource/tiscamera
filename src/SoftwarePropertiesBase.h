#pragma once

#include "base_types.h"

#include <map>
#include <string>
#include <string_view>

namespace tcam::property::emulated
{

enum class software_prop
{
    Invalid = 0,

    ExposureTime = 1,
    ExposureAuto = 2,
    ExposureAutoLowerLimit = 3,
    ExposureAutoUpperLimit = 4,
    ExposureAutoReference = 5,
    ExposureAutoUpperLimitAuto = 6,

    Gain = 7,
    GainAuto,
    GainAutoLowerLimit,
    GainAutoUpperLimit,

    Iris,
    IrisAuto,
    Focus,
    FocusAuto,

    HighlightReduction,

    WB,
    WB_AUTO,
    WB_RED,
    WB_GREEN,
    WB_BLUE,
    WB_ONCE,

    Denoise,
    Sharpness,

    Brightness,
    Contrast,
    Saturation,
    Hue,
    Gamma,
};


const std::string_view find_property_name(software_prop id);


struct software_prop_desc
{
    software_prop_desc(software_prop id, std::string_view n, TCAM_PROPERTY_TYPE t)
        : id_(id), name_(n), type_(t), range_d_(), range_i_(), entries_()
    {
    }
    software_prop_desc(software_prop id, std::string_view n, tcam_value_int r)
        : software_prop_desc(id, n, TCAM_PROPERTY_TYPE_INTEGER)
    {
        range_i_ = r;
    }
    software_prop_desc(software_prop id, std::string_view n, tcam_value_double r)
        : software_prop_desc(id, n, TCAM_PROPERTY_TYPE_DOUBLE)
    {
        range_d_ = r;
    }

    software_prop_desc(software_prop id, std::string_view n, const std::map<int, std::string>& e, int default_val)
        : software_prop_desc(id, n, TCAM_PROPERTY_TYPE_ENUMERATION)
    {
        //range_ = {};
        entries_ = e;
        default_value_ = default_val;
    }

    software_prop id_;
    std::string_view name_;
    TCAM_PROPERTY_TYPE type_;
    tcam_value_double range_d_;
    tcam_value_int range_i_;
    int default_value_ = 0;
    std::map<int, std::string> entries_;
};

} // namespace tcam::property::emulated
