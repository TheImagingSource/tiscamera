
#ifndef USER_PROPERTIES_H_
#define USER_PROPERTIES_H_

#include "base_types.h"

#include <string>
#include <vector>
#include <cstring>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

/**
   This file contains the mapping between userspace properties and device properties.
   Alle devices will have to abide to this mapping.
 */

namespace tcam
{

/*
  Reference table
  All controls will be mapped into this
  If a camera value is unknown it will be simply converted and shown unedited
*/
struct control_reference
{
    TCAM_PROPERTY_ID id;
    std::string name;               // name for external usage
    enum TCAM_PROPERTY_TYPE type_to_use; // type outgoing control shall have
    tcam_property_group group;
};

static const control_reference INVALID_STD_PROPERTY
{
    .id = TCAM_PROPERTY_INVALID,
    .name = "INVALID_PORPERTY",
    .type_to_use = TCAM_PROPERTY_TYPE_UNKNOWN,
    .group = { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID},
};


static const std::vector<struct control_reference> ctrl_reference_table =
{
    {
        .id = TCAM_PROPERTY_INVALID,
        .name = "INVALID_PORPERTY",
        .type_to_use = TCAM_PROPERTY_TYPE_UNKNOWN,
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE,
        .name = "Exposure",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE },
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO,
        .name = "Exposure Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE },
    },
    {
        .id = TCAM_PROPERTY_GAIN,
        .name = "Gain",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN },
    },
    {
        .id = TCAM_PROPERTY_GAIN_RED,
        .name = "Gain Red",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN },
    },
    {
        .id = TCAM_PROPERTY_GAIN_GREEN,
        .name = "Gain Green",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN },
    },
    {
        .id = TCAM_PROPERTY_GAIN_BLUE,
        .name = "Gain Blue",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN },
    },
    {
        .id = TCAM_PROPERTY_GAIN_AUTO,
        .name = "Gain Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_MODE,
        .name = "Trigger Mode",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_SOURCE,
        .name = "Trigger Source",
        .type_to_use = TCAM_PROPERTY_TYPE_STRING_TABLE,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_ACTIVATION,
        .name = "Trigger Activation",
        .type_to_use = TCAM_PROPERTY_TYPE_STRING_TABLE,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        .id = TCAM_PROPERTY_SOFTWARETRIGGER,
        .name = "Software Trigger",
        .type_to_use = TCAM_PROPERTY_TYPE_BUTTON,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        .id = TCAM_PROPERTY_GPIO,
        .name = "GPIO",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_GPIO },
    },
    {
        .id = TCAM_PROPERTY_GPIN,
        .name = "GPIn",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_GPIO },
    },
    {
        .id = TCAM_PROPERTY_GPOUT,
        .name = "GPOut",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_GPIO },
    },
    {
        .id = TCAM_PROPERTY_OFFSET_X,
        .name = "Offset X",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN, TCAM_PROPERTY_OFFSET_AUTO },
    },
    {
        .id = TCAM_PROPERTY_OFFSET_Y,
        .name = "Offset Y",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN, TCAM_PROPERTY_OFFSET_AUTO },
    },
    {
        .id = TCAM_PROPERTY_OFFSET_AUTO,
        .name = "Offset Auto Center",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN, TCAM_PROPERTY_OFFSET_AUTO },
    },
    {
        .id = TCAM_PROPERTY_BRIGHTNESS,
        .name = "Brightness",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_BRIGHTNESS },
    },
    {
        .id = TCAM_PROPERTY_CONTRAST,
        .name = "Contrast",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_CONTRAST },
    },
    {
        .id = TCAM_PROPERTY_SATURATION,
        .name = "Saturation",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_SATURATION },
    },
    {
        .id = TCAM_PROPERTY_HUE,
        .name = "Hue",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_HUE },
    },
    {
        .id = TCAM_PROPERTY_GAMMA,
        .name = "Gamma",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_IMAGE, TCAM_PROPERTY_GAMMA },
    },
    {
        .id = TCAM_PROPERTY_WB,
        .name = "Whitebalance",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_AUTO,
        .name = "Whitebalance Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_RED,
        .name = "Whitebalance Red",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_GREEN,
        .name = "Whitebalance Green",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_BLUE,
        .name = "Whitebalance Blue",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_IRCUT,
        .name = "IRCutFilter",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_IRCUT },
    },
    {
        .id = TCAM_PROPERTY_IRIS,
        .name = "Iris",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_IRIS },
    },
    {
        .id = TCAM_PROPERTY_FOCUS,
        .name = "Focus",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS },
    },
    {
        .id = TCAM_PROPERTY_ZOOM,
        .name = "Zoom",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_ZOOM },
    },
    {
        .id = TCAM_PROPERTY_FOCUS_AUTO,
        .name = "Focus Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS_AUTO },
    },
    {
        .id = TCAM_PROPERTY_FOCUS_ONE_PUSH,
        .name = "Focus One Push",
        .type_to_use = TCAM_PROPERTY_TYPE_BUTTON,
        .group = { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS },
    },
    {
        .id = TCAM_PROPERTY_STROBE_ENABLE,
        .name = "Strobe Enable",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE },
    },
    {
        .id = TCAM_PROPERTY_SKIPPING,
        .name = "Skipping",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_BINNING,
        .name = "Binning",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    // {
    //     .name = "Strobe Polarity",
    //     .type_to_use = ,
    // },
    // {
    //     .name = "Strobe Operation",
    //     .type_to_use = ,
    // },
    // {
    //     .name = "",
    //     .type_to_use = ,
    // },

};


inline control_reference get_control_reference (enum TCAM_PROPERTY_ID wanted_id)
{
    for (const auto& ref : ctrl_reference_table)
    {
        if (ref.id == wanted_id)
            return ref;
    }
    return INVALID_STD_PROPERTY;
}



inline tcam_camera_property create_empty_property (enum TCAM_PROPERTY_ID id)
{
    auto ref = get_control_reference(id);

    tcam_camera_property prop = {};
    prop.type = ref.type_to_use;
    strncpy(prop.name, ref.name.c_str(), sizeof(prop.name));
    prop.id = ref.id;
    prop.group = ref.group;

    return prop;
}

} /*namespace tcam */

VISIBILITY_POP

#endif /* user_properties */
