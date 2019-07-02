/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#ifndef TCAM_STANDARD_PROPERTIES_H
#define TCAM_STANDARD_PROPERTIES_H

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

    control_reference (TCAM_PROPERTY_ID id_,
                       const std::string& name_,
                       enum TCAM_PROPERTY_TYPE type_to_use_,
                       tcam_property_group group_)
        : id(id_), name(name_), type_to_use(type_to_use_), group(group_)
    { }
};

static const control_reference INVALID_STD_PROPERTY (TCAM_PROPERTY_INVALID,
                                                     "INVALID_PORPERTY",
                                                     TCAM_PROPERTY_TYPE_UNKNOWN,
                                                     { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0});
// {
//     .id = TCAM_PROPERTY_INVALID,
//     .name = "INVALID_PORPERTY",
//     .type_to_use = TCAM_PROPERTY_TYPE_UNKNOWN,
//     { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0},
// };


static const std::vector<struct control_reference> ctrl_reference_table =
{
    {
        TCAM_PROPERTY_INVALID,
        "INVALID_PORPERTY",
        TCAM_PROPERTY_TYPE_UNKNOWN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_EXPOSURE,
        "Exposure",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE, 0 },
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO,
        "Exposure Auto",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE, 0 },
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_REFERENCE,
        "Exposure Auto Reference",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE, 0 },
    },
    {
        TCAM_PROPERTY_HIGHLIGHT_REDUCTION,
        "Highlight Reduction",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE, 0 },
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT_AUTO,
        "Exposure Auto Upper Limit Auto",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE, 0 },
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT,
        "Exposure Auto Upper Limit",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE, 0 },
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_LOWER_LIMIT,
        "Exposure Auto Lower Limit",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE, 0 },
    },
    {
        TCAM_PROPERTY_GAIN,
        "Gain",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN, 0 },
    },
    {
        TCAM_PROPERTY_GAIN_RED,
        "Gain Red",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN, 0 },
    },
    {
        TCAM_PROPERTY_GAIN_GREEN,
        "Gain Green",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN, 0 },
    },
    {
        TCAM_PROPERTY_GAIN_BLUE,
        "Gain Blue",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN, 0 },
    },
    {
        TCAM_PROPERTY_GAIN_AUTO,
        "Gain Auto",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN, 0 },
    },
    {
        TCAM_PROPERTY_GAIN_AUTO_UPPER_LIMIT,
        "Gain Auto Upper Limit",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN, 0 },
    },
    {
        TCAM_PROPERTY_GAIN_AUTO_LOWER_LIMIT,
        "Gain Auto Lower Limit",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_MODE,
        "Trigger Mode",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_SOURCE,
        "Trigger Source",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_ACTIVATION,
        "Trigger Activation",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_SOFTWARETRIGGER,
        "Software Trigger",
        TCAM_PROPERTY_TYPE_BUTTON,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_DENOISE,
        "Trigger Denoise",
        TCAM_PROPERTY_TYPE_DOUBLE,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_MASK,
        "Trigger Mask",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_DEBOUNCER,
        "Trigger Debouncer",
        TCAM_PROPERTY_TYPE_DOUBLE,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_DELAY,
        "Trigger Delay (us)",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_SELECTOR,
        "Trigger Selector",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_OPERATION,
        "Trigger Operation",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_POLARITY,
        "Trigger Polarity",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_EXPOSURE_MODE,
        "Trigger Exposure Mode",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_BURST_COUNT,
        "Trigger Burst Count",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_DEBOUNCE_TIME_US,
        "Trigger Debounce Time (us)",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_MASK_TIME_US,
        "Trigger Mask Time (us)",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_TRIGGER_NOISE_SURPRESSION_TIME_US,
        "Trigger Noise Surpression Time (us)",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
    {
        TCAM_PROPERTY_GPIO,
        "GPIO",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_GPIO, 0 },
    },
    {
        TCAM_PROPERTY_GPIN,
        "GPIn",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_GPIO, 0 },
    },
    {
        TCAM_PROPERTY_GPOUT,
        "GPOut",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_GPIO, 0 },
    },
    {
        TCAM_PROPERTY_OFFSET_X,
        "Offset X",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN, TCAM_PROPERTY_OFFSET_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_OFFSET_Y,
        "Offset Y",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN, TCAM_PROPERTY_OFFSET_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_OFFSET_AUTO,
        "Offset Auto Center",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN, TCAM_PROPERTY_OFFSET_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_BRIGHTNESS,
        "Brightness",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_BRIGHTNESS, 0 },
    },
    {
        TCAM_PROPERTY_CONTRAST,
        "Contrast",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_CONTRAST, 0 },
    },
    {
        TCAM_PROPERTY_SATURATION,
        "Saturation",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_SATURATION, 0 },
    },
    {
        TCAM_PROPERTY_HUE,
        "Hue",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_HUE, 0 },
    },
    {
        TCAM_PROPERTY_GAMMA,
        "Gamma",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_IMAGE, TCAM_PROPERTY_GAMMA, 0 },
    },
    {
        TCAM_PROPERTY_WB,
        "Whitebalance",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_AUTO,
        "Whitebalance Auto",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_RED,
        "Whitebalance Red",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_GREEN,
        "Whitebalance Green",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_BLUE,
        "Whitebalance Blue",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_TEMPERATURE,
        "Whitebalance Temperature",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_AUTO_PRESET,
        "Whitebalance Auto Preset",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_ONCE,
        "Whitebalance Once",
        TCAM_PROPERTY_TYPE_BUTTON,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_MODE,
        "Whitebalance Mode",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_PRESET,
        "Whitebalance Preset",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_WB_TEMPERATURE,
        "Whitebalance Temerature",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_BALANCERATIO_SELECTOR,
        "Balance Ratio Selector",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_BALANCERATIO,
        "Balance Ratio",
        TCAM_PROPERTY_TYPE_DOUBLE,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_BALANCE_WHITE_AUTO_PRESET,
        "White Balance Auto Preset",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_BALANCE_WHITE_TEMPERATURE_PRESET,
        "White Balance Temperature Preset",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB, 0 },
    },
    {
        TCAM_PROPERTY_IRCUT,
        "IRCutFilter",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_IRCUT, 0 },
    },
    {
        TCAM_PROPERTY_IRIS,
        "Iris",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_IRIS, 0 },
    },
    {
        TCAM_PROPERTY_FOCUS,
        "Focus",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS, 0 },
    },
    {
        TCAM_PROPERTY_ZOOM,
        "Zoom",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_ZOOM, 0 },
    },
    {
        TCAM_PROPERTY_FOCUS_AUTO,
        "Focus Auto",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_FOCUS_ROI_TOP,
        "Focus ROI Top",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_FOCUS_ROI_LEFT,
        "Focus ROI Left",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_FOCUS_ROI_WIDTH,
        "Focus ROI Width",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_FOCUS_ROI_HEIGHT,
        "Focus ROI Height",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS_AUTO, 0 },
    },
    {
        TCAM_PROPERTY_FOCUS_ONE_PUSH,
        "Focus One Push",
        TCAM_PROPERTY_TYPE_BUTTON,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_FOCUS, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_ENABLE,
        "Strobe Enable",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_EXPOSURE,
        "Strobe Exposure",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_DURATION,
        "Strobe Duration",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_OPERATION,
        "Strobe Operation",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_POLARITY,
        "Strobe Polarity",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_DELAY,
        "Strobe Delay",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_DELAY_SECOND,
        "Strobe Second Delay",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_DURATION,
        "Strobe Duration",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_DURATION_SECOND,
        "Strobe Second Duration",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_STROBE_MODE,
        "Strobe Mode",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE, 0 },
    },
    {
        TCAM_PROPERTY_SKIPPING,
        "Skipping",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_BINNING,
        "Binning",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_BINNING_HORIZONTAL,
        "Binning Horizontal",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_BINNING_VERTICAL,
        "Binning Vertical",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_SHARPNESS,
        "Sharpness",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_REVERSE_X,
        "Reverse X",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_REVERSE_Y,
        "Reverse Y",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_BLACKLEVEL,
        "Black Level",
        TCAM_PROPERTY_TYPE_DOUBLE,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_CHUNK_MODE_ACTIVE,
        "Chunk Mode Active",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_COUNT,
        "Stream Channel Count",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_SELECTOR,
        "Stream Channel Selector",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_TYPE,
        "Stream Channel Type",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_LINK,
        "Stream Channel Link",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_ENDIANNESS,
        "Stream Channel Endianness",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_PACKET_SIZE,
        "Stream Channel Packet Size",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_EVENT_CHANNEL_COUNT,
        "Event Channel Count",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_PAYLOAD_SIZE,
        "Payload Size",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_PAYLOAD_PER_FRAME,
        "Payload Per Frame",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_PAYLOAD_PER_PACKET,
        "Payload Per Packet",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_TOTAL_PACKET_SIZE,
        "Total Packet Size",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_PACKETS_PER_FRAME,
        "Packets Per Frame",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_PACKET_TIME_US,
        "Packet Time (1000ns)",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_NOISE_REDUCTION,
        "Noise Reduction",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_FACE_DETECTION,
        "Face Detection",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_IMAGE_STABILIZATION,
        "Image Stabilization",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_CHUNK_IMX174_FRAME_ID,
        "IMX174 Frame ID",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_CHUNK_IMX174_FRAME_SET,
        "IMX174 Frame Set",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_IMX174_WDR_SHUTTER2,
        "IMX174 WDR Shutter 2",
        TCAM_PROPERTY_TYPE_DOUBLE,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_IMX174_HARDWARE_WDR_ENABLE,
        "IMX174 WDR Enable",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_UNKNOWN, TCAM_PROPERTY_INVALID, 0 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL,
        "Auto Functions ROI Control",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL, 0 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_LEFT,
        "Auto Functions ROI Left",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL, 0 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_TOP,
        "Auto Functions ROI Top",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL, 0 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_WIDTH,
        "Auto Functions ROI Width",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL, 0 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_HEIGHT,
        "Auto Functions ROI Height",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL, 0 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_PRESET,
        "Auto Functions ROI Preset",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL, 0 },
    },
    {
        TCAM_PROPERTY_SHUTTER,
        "Shutter",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_SHUTTER, 0 },
    },
    {
        TCAM_PROPERTY_HDR,
        "HDR",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_HDR, 0 },
    },
    {
        TCAM_PROPERTY_OIS_MODE,
        "OIS Mode",
        TCAM_PROPERTY_TYPE_ENUMERATION,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_OIS_MODE, 0 },
    },
    {
        TCAM_PROPERTY_OIS_POS_X,
        "OIS Item Position X",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_OIS_MODE, 0 },
    },
    {
        TCAM_PROPERTY_OIS_MODE,
        "OIS Item Position Y",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_LENS, TCAM_PROPERTY_OIS_MODE, 0 },
    },
    {
        TCAM_PROPERTY_OVERRIDE_SCANNING_MODE,
        "Override Scanning Mode",
        TCAM_PROPERTY_TYPE_INTEGER,
        { TCAM_PROPERTY_CATEGORY_PARTIAL_SCAN, TCAM_PROPERTY_OVERRIDE_SCANNING_MODE, 0 },
    },
    {
        TCAM_PROPERTY_IMX_LOW_LATENCY_MODE,
        "IMX Low-Latency Mode",
        TCAM_PROPERTY_TYPE_BOOLEAN,
        { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE, 0 },
    },
};


inline control_reference get_control_reference (TCAM_PROPERTY_ID wanted_id)
{
    for (const auto& ref : ctrl_reference_table)
    {
        if (ref.id == wanted_id)
            return ref;
    }
    return INVALID_STD_PROPERTY;
}


inline TCAM_PROPERTY_ID string2property_id (const std::string& name)
{
    for (const auto& ref : ctrl_reference_table)
    {
        if (name.compare(ref.name) == 0)
        {
            return ref.id;
        }
    }

    return TCAM_PROPERTY_INVALID;
}


inline std::string property_id2string (TCAM_PROPERTY_ID id)
{
    for (const auto& ref : ctrl_reference_table)
    {
        if (ref.id == id)
        {
            return ref.name;
        }
    }
    return "";
}


inline tcam_device_property create_empty_property (TCAM_PROPERTY_ID id)
{
    auto ref = get_control_reference(id);

    tcam_device_property prop = {};
    prop.type = ref.type_to_use;
    strncpy(prop.name, ref.name.c_str(), sizeof(prop.name) - 1);
    prop.id = ref.id;
    prop.group = ref.group;

    return prop;
}


inline TCAM_PROPERTY_TYPE get_reference_property_type (TCAM_PROPERTY_ID id)
{
    return get_control_reference(id).type_to_use;
}


} /*namespace tcam */

VISIBILITY_POP

#endif /* TCAM_STANDARD_PROPERTIES_H */
