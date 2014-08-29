
#ifndef USER_PROPERTIES_H_
#define USER_PROPERTIES_H_

#include "base_types.h"

#include <linux/videodev2.h>
#include <string>
#include <vector>

/**
   This file contains the mapping between userspace properties and device properties.
   Alle devices will have to abide to this mapping.
 */

namespace tis_imaging
{

/*
  Reference table
  All controls will be mapped into this
  If a camera value is unknown it will be simply converted and shown unedited
*/
struct control_reference
{
    std::string name;               // name for external usage
    enum PROPERTY_TYPE type_to_use; // type outgoing control shall have

    std::vector<int> v4l2_id;              // list of v4l2 identifiers that shall be mapped to control
    std::vector<std::string> v4l2_name;
    std::vector<std::string> genicam_name; // list of genicam identifiers that shall be mapped to control
};


static std::vector<struct control_reference> ctrl_reference_table =
{
    {
        .name = "Exposure",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = { V4L2_CID_EXPOSURE_ABSOLUTE, V4L2_CID_EXPOSURE },
        .v4l2_name = { "Exposure (Absolute)"},
        .genicam_name = {"ExposureTime"},
    },
    {
        .name = "Exposure Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = { V4L2_CID_AUTO_EXPOSURE_BIAS },
        .v4l2_name = {},
        .genicam_name = {"ExposureAuto"},
    },
    {
        "Gain",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = { V4L2_CID_GAIN },
        .v4l2_name = { "Gain" },
        .genicam_name = {"Gain"},
    },
    {
        "Gain Red",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = { 0 },
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        "Gain Green",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = { 0 },
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        "Gain Blue",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {0 },
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        "Gain Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = {0 },
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        .name = "Trigger Mode",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = { V4L2_CID_PRIVACY},
        .v4l2_name = { "Privacy" },
        .genicam_name = {"TriggerMode"},
    },
    {
        .name = "Trigger Source",
        .type_to_use = PROPERTY_TYPE_STRING_TABLE,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"TriggerSource"},
    },
    {
        // enum
        .name = "Trigger Activation",
        .type_to_use = PROPERTY_TYPE_STRING_TABLE,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"TriggerActivation"},
    },
    {
        .name = "Software Trigger",
        .type_to_use = PROPERTY_TYPE_BUTTON,
        .v4l2_id = {0},
        .v4l2_name = { "SoftwareTrigger" },
        .genicam_name = {"TriggerSoftware"},
    },
    {
        .name = "GPIO",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"GPIO"},
    },
    {
        .name = "GPIn",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"GPIn"},
    },
    {
        .name = "GPOut",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"GPOut"},
    },
    {
        .name = "Offset X",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"OffsetX"},
    },
    {
        .name = "Offset Y",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"OffsetY"},
    },
    {
        .name = "Offset Auto Center",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = {0},
        .v4l2_name = {},
        .genicam_name = {"OffsetAutoCenter"},
    },
    {
        .name = "Brightness",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {V4L2_CID_BRIGHTNESS},
        .v4l2_name = { "Brightness" },
        .genicam_name = {},
    },
    {
        .name = "Contrast",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {V4L2_CID_CONTRAST},
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        .name = "Saturation",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {V4L2_CID_SATURATION},
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        .name = "Hue",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {V4L2_CID_HUE},
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        .name = "Gamma",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {V4L2_CID_GAMMA},
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        .name = "Whitebalance Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = {},
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        .name = "IRCutFilter",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = {},
        .v4l2_name = {},
        .genicam_name = {"IRCutFilter"},
    },
    {
        .name = "Iris",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {},
        .v4l2_name = {},
        .genicam_name = {"Iris"},
    },
    {
        .name = "Focus",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {},
        .v4l2_name = { "Focus (Absolute)" },
        .genicam_name = {"Focus"},
    },
    {
        .name = "Zoom",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .v4l2_id = {},
        .v4l2_name = { "Zoom, Absolute" },
        .genicam_name = {"Zoom"},
    },
    {
        .name = "Focus Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = {},
        .v4l2_name = {},
        .genicam_name = {},
    },
    {
        .name = "Strobe Enable",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .v4l2_id = {},
        .v4l2_name = {},
        .genicam_name = {"StrobeEnable"},
    },
    // {
    //     .name = "Strobe Polarity",
    //     .type_to_use = ,
    //     .v4l2_id = {},
    //     .genicam_name = {"StrobePolarity"},
    // },
    // {
    //     .name = "Strobe Operation",
    //     .type_to_use = ,
    //     .v4l2_id = {},
    //     .genicam_name = {"StrobeOperation"},
    // },
    // {
    //     .name = "",
    //     .type_to_use = ,
    //     .v4l2_id = {},
    //     .genicam_name = {},
    // },

};

} /*namespace tis_imaging */

#endif /* user_properties */
