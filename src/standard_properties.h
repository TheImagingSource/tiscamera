
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

enum PROPERTY_ID
{
    PROPERTY_INVALID,
    PROPERTY_EXPOSURE,
    PROPERTY_EXPOSURE_AUTO,
    PROPERTY_GAIN,
    PROPERTY_GAIN_RED,
    PROPERTY_GAIN_GREEN,
    PROPERTY_GAIN_BLUE,
    PROPERTY_GAIN_AUTO,
    PROPERTY_TRIGGER_MODE,
    PROPERTY_TRIGGER_SOURCE,
    PROPERTY_TRIGGER_ACTIVATION,
    PROPERTY_SOFTWARETRIGGER,
    PROPERTY_GPIO,
    PROPERTY_GPIN,
    PROPERTY_GPOUT,
    PROPERTY_OFFSET_X,
    PROPERTY_OFFSET_Y,
    PROPERTY_OFFSET_AUTO,
    PROPERTY_BRIGHTNESS,
    PROPERTY_CONTRAST,
    PROPERTY_SATURATION,
    PROPERTY_HUE,
    PROPERTY_GAMMA,
    PROPERTY_WB_AUTO,
    PROPERTY_IRCUT,
    PROPERTY_IRIS,
    PROPERTY_FOCUS,
    PROPERTY_ZOOM,
    PROPERTY_FOCUS_AUTO,
    PROPERTY_STROBE_ENABLE,
};

/*
  Reference table
  All controls will be mapped into this
  If a camera value is unknown it will be simply converted and shown unedited
*/
struct control_reference
{
    PROPERTY_ID id;
    std::string name;               // name for external usage
    enum PROPERTY_TYPE type_to_use; // type outgoing control shall have

    std::vector<std::string> genicam_name; // list of genicam identifiers that shall be mapped to control
};


static std::vector<struct control_reference> ctrl_reference_table =
{
    {
        .id = PROPERTY_INVALID,
        .name = "INVALID_PORPERTY",
        .type_to_use = PROPERTY_TYPE_UNKNOWN,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_EXPOSURE,
        .name = "Exposure",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"ExposureTime"},
    },
    {
        .id = PROPERTY_EXPOSURE_AUTO,
        .name = "Exposure Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {"ExposureAuto"},
    },
    {
        .id = PROPERTY_GAIN,
        .name = "Gain",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"Gain"},
    },
    {
        .id = PROPERTY_GAIN_RED,
        "Gain Red",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_GAIN_GREEN,
        "Gain Green",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_GAIN_BLUE,
        "Gain Blue",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_GAIN_AUTO,
        "Gain Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_TRIGGER_MODE,
        .name = "Trigger Mode",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {"TriggerMode"},
    },
    {
        .id = PROPERTY_TRIGGER_SOURCE,
        .name = "Trigger Source",
        .type_to_use = PROPERTY_TYPE_STRING_TABLE,
        .genicam_name = {"TriggerSource"},
    },
    {
        .id = PROPERTY_TRIGGER_ACTIVATION,
        // enum
        .name = "Trigger Activation",
        .type_to_use = PROPERTY_TYPE_STRING_TABLE,
        .genicam_name = {"TriggerActivation"},
    },
    {
        .id = PROPERTY_SOFTWARETRIGGER,
        .name = "Software Trigger",
        .type_to_use = PROPERTY_TYPE_BUTTON,
        .genicam_name = {"TriggerSoftware"},
    },
    {
        .id = PROPERTY_GPIO,
        .name = "GPIO",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"GPIO"},
    },
    {
        .id = PROPERTY_GPIN,
        .name = "GPIn",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"GPIn"},
    },
    {
        .id = PROPERTY_GPOUT,
        .name = "GPOut",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"GPOut"},
    },
    {
        .id = PROPERTY_OFFSET_X,
        // TODO extract extension unit ids
        .name = "Offset X",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"OffsetX"},
    },
    {
        .id = PROPERTY_OFFSET_Y,
        .name = "Offset Y",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"OffsetY"},
    },
    {
        .id = PROPERTY_OFFSET_AUTO,
        .name = "Offset Auto Center",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {"OffsetAutoCenter"},
    },
    {
        .id = PROPERTY_BRIGHTNESS,
        .name = "Brightness",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_CONTRAST,
        .name = "Contrast",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_SATURATION,
        .name = "Saturation",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_HUE,
        .name = "Hue",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_GAMMA,
        .name = "Gamma",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_WB_AUTO,
        .name = "Whitebalance Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_IRCUT,
        .name = "IRCutFilter",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {"IRCutFilter"},
    },
    {
        .id = PROPERTY_IRIS,
        .name = "Iris",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"Iris"},
    },
    {
        .id = PROPERTY_FOCUS,
        .name = "Focus",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"Focus"},
    },
    {
        .id = PROPERTY_ZOOM,
        .name = "Zoom",
        .type_to_use = PROPERTY_TYPE_INTEGER,
        .genicam_name = {"Zoom"},
    },
    {
        .id = PROPERTY_FOCUS_AUTO,
        .name = "Focus Auto",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {},
    },
    {
        .id = PROPERTY_STROBE_ENABLE,
        .name = "Strobe Enable",
        .type_to_use = PROPERTY_TYPE_BOOLEAN,
        .genicam_name = {"StrobeEnable"},
    },
    // {
    //     .name = "Strobe Polarity",
    //     .type_to_use = ,
    //     .genicam_name = {"StrobePolarity"},
    // },
    // {
    //     .name = "Strobe Operation",
    //     .type_to_use = ,
    //     .genicam_name = {"StrobeOperation"},
    // },
    // {
    //     .name = "",
    //     .type_to_use = ,
    //     .genicam_name = {},
    // },

};


inline control_reference get_control_reference (enum PROPERTY_ID wanted_id)
{
    for (const auto& ref : ctrl_reference_table)
    {
        if (ref.id == wanted_id)
            return ref;
    }
    return {PROPERTY_INVALID, "", PROPERTY_TYPE_UNKNOWN, {}};
}


} /*namespace tis_imaging */

#endif /* user_properties */
