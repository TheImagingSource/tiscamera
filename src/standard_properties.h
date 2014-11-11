
#ifndef USER_PROPERTIES_H_
#define USER_PROPERTIES_H_

#include "base_types.h"
#include "Error.h"

#include <linux/videodev2.h>
#include <string>
#include <vector>
#include <cstring>

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
    PROPERTY_ID id;
    std::string name;               // name for external usage
    enum TCAM_PROPERTY_TYPE type_to_use; // type outgoing control shall have
};

static control_reference INVALID_STD_PROPERTY
{
    .id = PROPERTY_INVALID,
    .name = "INVALID_PORPERTY",
    .type_to_use = TCAM_PROPERTY_TYPE_UNKNOWN
};


static std::vector<struct control_reference> ctrl_reference_table =
{
    {
        .id = PROPERTY_INVALID,
        .name = "INVALID_PORPERTY",
        .type_to_use = TCAM_PROPERTY_TYPE_UNKNOWN,
    },
    {
        .id = PROPERTY_EXPOSURE,
        .name = "Exposure",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_EXPOSURE_AUTO,
        .name = "Exposure Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_GAIN,
        .name = "Gain",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_GAIN_RED,
        .name = "Gain Red",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_GAIN_GREEN,
        .name = "Gain Green",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_GAIN_BLUE,
        .name = "Gain Blue",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_GAIN_AUTO,
        .name = "Gain Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_TRIGGER_MODE,
        .name = "Trigger Mode",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_TRIGGER_SOURCE,
        .name = "Trigger Source",
        .type_to_use = TCAM_PROPERTY_TYPE_STRING_TABLE,
    },
    {
        .id = PROPERTY_TRIGGER_ACTIVATION,
        // enum
        .name = "Trigger Activation",
        .type_to_use = TCAM_PROPERTY_TYPE_STRING_TABLE,
    },
    {
        .id = PROPERTY_SOFTWARETRIGGER,
        .name = "Software Trigger",
        .type_to_use = TCAM_PROPERTY_TYPE_BUTTON,
    },
    {
        .id = PROPERTY_GPIO,
        .name = "GPIO",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_GPIN,
        .name = "GPIn",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_GPOUT,
        .name = "GPOut",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_OFFSET_X,
        .name = "Offset X",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_OFFSET_Y,
        .name = "Offset Y",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_OFFSET_AUTO,
        .name = "Offset Auto Center",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_BRIGHTNESS,
        .name = "Brightness",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_CONTRAST,
        .name = "Contrast",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_SATURATION,
        .name = "Saturation",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_HUE,
        .name = "Hue",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_GAMMA,
        .name = "Gamma",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_WB,
        .name = "Whitebalance",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_WB_AUTO,
        .name = "Whitebalance Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_WB_RED,
        .name = "Whitebalance Red",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_WB_GREEN,
        .name = "Whitebalance Green",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_WB_BLUE,
        .name = "Whitebalance Blue",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_IRCUT,
        .name = "IRCutFilter",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_IRIS,
        .name = "Iris",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_FOCUS,
        .name = "Focus",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_ZOOM,
        .name = "Zoom",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        .id = PROPERTY_FOCUS_AUTO,
        .name = "Focus Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_STROBE_ENABLE,
        .name = "Strobe Enable",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        .id = PROPERTY_SKIPPING,
        .name = "Skipping",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        PROPERTY_BINNING,
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


inline control_reference get_control_reference (enum PROPERTY_ID wanted_id)
{
    for (const auto& ref : ctrl_reference_table)
    {
        if (ref.id == wanted_id)
            return ref;
    }
    return INVALID_STD_PROPERTY;
}



inline camera_property create_empty_property (enum PROPERTY_ID id)
{
    auto ref = get_control_reference(id);
    if (ref.id == INVALID_STD_PROPERTY.id)
    {
        setError(Error("No matching property", ENOENT));
    }

    camera_property prop = {};
    prop.type = ref.type_to_use;
    strncpy(prop.name, ref.name.c_str(), sizeof(prop.name));

    return prop;
}

} /*namespace tcam */

#endif /* user_properties */
