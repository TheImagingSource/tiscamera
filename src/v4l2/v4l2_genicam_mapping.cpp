
#include "v4l2_genicam_mapping.h"

#include <map>



static const std::map<int, struct tcam::v4l2::v4l2_genicam_mapping> v4l2_conv_dict
{
    {
        0x009a0902, // exposure absolute
        {
            "ExposureTime",
            TCAM_PROPERTY_TYPE_DOUBLE,
        },
    },
    {
        0x199e205, // Gain Auto
        {
            "GainAuto",
            TCAM_PROPERTY_TYPE_ENUMERATION,
        }
    },
    {
        0x199e202, // Auto Shutter
        {
            "ExposureAuto",
            TCAM_PROPERTY_TYPE_ENUMERATION,
        }
    },
    {
        0x199e201, // Exposure Time (us)
        {
            "ExposureTime",
            TCAM_PROPERTY_TYPE_DOUBLE,
        }
    },
    {
        0x199e209,
        {
            "TriggerSoftware",
            TCAM_PROPERTY_TYPE_UNKNOWN,
        },
    },
    {
        0x199e208,
        {
            "TriggerMode",
            TCAM_PROPERTY_TYPE_ENUMERATION,
        },
    },
};


const struct tcam::v4l2::v4l2_genicam_mapping* tcam::v4l2::find_mapping(uint32_t v4l2_id)
{
    auto it = v4l2_conv_dict.find(v4l2_id);

    if (it == v4l2_conv_dict.end())
    {
        return nullptr;
    }

    return &(it->second);
}
