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
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_REFERENCE,
        .name = "Exposure Auto Reference",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE },
    },
    {
        .id = TCAM_PROPERTY_HIGHLIGHT_REDUCTION,
        .name = "Highlight Reduction",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE },
    },
    {
        .id = TCAM_PROPERTY_AUTO_REFERENCE,
        .name = "Exposure Auto Reference",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE },
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT_AUTO,
        .name = "Exposure Auto Upper Limit Auto",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE },
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT,
        .name = "Exposure Auto Upper Limit",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_EXPOSURE },
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_LOWER_LIMIT,
        .name = "Exposure Auto Lower Limit",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
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
        .id = TCAM_PROPERTY_GAIN_AUTO_UPPER_LIMIT,
        .name = "Gain Auto Upper Limit",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_EXPOSURE, TCAM_PROPERTY_GAIN },
    },
    {
        .id = TCAM_PROPERTY_GAIN_AUTO_LOWER_LIMIT,
        .name = "Gain Auto Lower Limit",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
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
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_ACTIVATION,
        .name = "Trigger Activation",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        .id = TCAM_PROPERTY_SOFTWARETRIGGER,
        .name = "Software Trigger",
        .type_to_use = TCAM_PROPERTY_TYPE_BUTTON,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_DENOISE,
        .name = "Trigger Denoise",
        .type_to_use = TCAM_PROPERTY_TYPE_DOUBLE,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_MASK,
        .name = "Trigger Mask",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_DEBOUNCER,
        .name = "Trigger Debouncer",
        .type_to_use = TCAM_PROPERTY_TYPE_DOUBLE,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_DELAY,
        .name = "Trigger Delay (us)",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_SELECTOR,
        .name = "Trigger Selector",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_OPERATION,
        .name = "Trigger Operation",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_POLARITY,
        .name = "Trigger Polarity",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_EXPOSURE_MODE,
        .name = "Trigger Exposure Mode",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_BURST_COUNT,
        .name = "Trigger Burst Count",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_DEBOUNCE_TIME_US,
        .name = "Trigger Debounce Time (us)",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_MASK_TIME_US,
        .name = "Trigger Mask Time (us)",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_TRIGGER_MODE },
    },
    {
        TCAM_PROPERTY_TRIGGER_NOISE_SURPRESSION_TIME_US,
        .name = "Trigger Noise Surpression Time (us)",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
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
        .id = TCAM_PROPERTY_WB_TEMPERATURE,
        .name = "Whitebalance Temperature",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_AUTO_PRESET,
        .name = "Whitebalance Auto Preset",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_ONCE,
        .name = "Whitebalance Once",
        .type_to_use = TCAM_PROPERTY_TYPE_BUTTON,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_MODE,
        .name = "Whitebalance Mode",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_PRESET,
        .name = "Whitebalance Preset",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_WB_TEMPERATURE,
        .name = "Whitebalance Temerature",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB },
    },
    {
        .id = TCAM_PROPERTY_BALANCERATIO_SELECTOR,
        .name = "Balance Ratio Selector",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB},
    },
    {
        .id = TCAM_PROPERTY_BALANCERATIO,
        .name = "Balance Ratio",
        .type_to_use = TCAM_PROPERTY_TYPE_DOUBLE,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB},
    },
    {
        .id = TCAM_PROPERTY_BALANCE_WHITE_AUTO_PRESET,
        .name = "White Balance Auto Preset",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB},
    },
    {
        .id = TCAM_PROPERTY_BALANCE_WHITE_TEMPERATURE_PRESET,
        .name = "White Balance Temperature Preset",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_COLOR, TCAM_PROPERTY_WB},
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
        .id = TCAM_PROPERTY_STROBE_EXPOSURE,
        .name = "Strobe Exposure",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE },
    },
    {
        .id = TCAM_PROPERTY_STROBE_DURATION,
        .name = "Strobe Duration",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE },
    },
    {
        .id = TCAM_PROPERTY_STROBE_OPERATION,
        .name = "Strobe Operation",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE },
    },
    {
        .id = TCAM_PROPERTY_STROBE_POLARITY,
        .name = "Strobe Polarity",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE },
    },
    {
        .id = TCAM_PROPERTY_STROBE_DELAY,
        .name = "Strobe Delay",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_STROBE_ENABLE },
    },
    {
        .id = TCAM_PROPERTY_STROBE_DURATION,
        .name = "Strobe Duration",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
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
    {
        TCAM_PROPERTY_BINNING_HORIZONTAL,
        .name = "Binning Horizontal",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
    },
    {
        TCAM_PROPERTY_BINNING_VERTICAL,
        .name = "Binning Vertical",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
    },
    {
        TCAM_PROPERTY_SHARPNESS,
        .name = "Sharpness",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_REVERSE_X,
        .name = "Reverse X",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_REVERSE_Y,
        .name = "Reverse Y",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_BLACKLEVEL,
        .name = "Black Level",
        .type_to_use = TCAM_PROPERTY_TYPE_DOUBLE,
    },
    {
        TCAM_PROPERTY_CHUNK_MODE_ACTIVE,
        .name = "Chunk Mode Active",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_COUNT,
        .name = "Stream Channel Count",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_SELECTOR,
        .name = "Stream Channel Selector",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_TYPE,
        .name = "Stream Channel Type",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_LINK,
        .name = "Stream Channel Link",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_ENDIANNESS,
        .name = "Stream Channel Endianness",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_PACKET_SIZE,
        .name = "Stream Channel Packet Size",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_EVENT_CHANNEL_COUNT,
        .name = "Event Channel Count",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_PAYLOAD_SIZE,
        .name = "Payload Size",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_PAYLOAD_PER_FRAME,
        .name = "Payload Per Frame",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_PAYLOAD_PER_PACKET,
        .name = "Payload Per Packet",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_TOTAL_PACKET_SIZE,
        .name = "Total Packet Size",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_PACKETS_PER_FRAME,
        .name = "Packets Per Frame",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_PACKET_TIME_US,
        .name = "Packet Time (1000ns)",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_NOISE_REDUCTION,
        .name = "Noise Reduction",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_FACE_DETECTION,
        .name = "Face Detection",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_IMAGE_STABILIZATION,
        .name = "Image Stabilization",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_CHUNK_IMX174_FRAME_ID,
        .name = "IMX174 Frame ID",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_CHUNK_IMX174_FRAME_SET,
        .name = "IMX174 Frame Set",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
    },
    {
        TCAM_PROPERTY_IMX174_WDR_SHUTTER2,
        .name = "IMX174 WDR Shutter 2",
        .type_to_use = TCAM_PROPERTY_TYPE_DOUBLE,
    },
    {
        TCAM_PROPERTY_IMX174_HARDWARE_WDR_ENABLE,
        .name = "IMX174 WDR Enable",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL,
        .name = "Auto Functions ROI Control",
        .type_to_use = TCAM_PROPERTY_TYPE_BOOLEAN,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_LEFT,
        .name = "Auto Functions ROI Left",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_TOP,
        .name = "Auto Functions ROI Top",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_WIDTH,
        .name = "Auto Functions ROI Width",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_HEIGHT,
        .name = "Auto Functions ROI Height",
        .type_to_use = TCAM_PROPERTY_TYPE_INTEGER,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_PRESET,
        .name = "Auto Functions ROI Preset",
        .type_to_use = TCAM_PROPERTY_TYPE_ENUMERATION,
        .group = { TCAM_PROPERTY_CATEGORY_SPECIAL, TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL },
    },
    // {
    //     TCAM_PROPERTY_,
    //     .name = "",
    //     .type_to_use = TCAM_PROPERTY_TYPE,
    // },

    // {
    //     .name = "Strobe Polarity",
    //     .type_to_use = ,
    // },
    // {
    //     .name = "Strobe Operation",
    //     .type_to_use = ,
    // },

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
    strncpy(prop.name, ref.name.c_str(), sizeof(prop.name));
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
