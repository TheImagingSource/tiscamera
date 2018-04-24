/*
 * Copyright 2017 The Imaging Source Europe GmbH
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

#ifndef TCAM_V4L2_PROPERTY_MAPPING_H
#define TCAM_V4L2_PROPERTY_MAPPING_H

#include "base_types.h"
#include "standard_properties.h"
#include "v4l2_uvc_identifier.h"
#include <linux/videodev2.h>

#include <vector>

struct v4l2_property
{
    TCAM_PROPERTY_ID id;
    std::vector<int> v4l2_id;
};


static const std::vector<struct v4l2_property> v4l2_mappings =
{
    {
        .id = TCAM_PROPERTY_INVALID,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE,
        .v4l2_id = { V4L2_CID_EXPOSURE_ABSOLUTE, V4L2_CID_EXPOSURE, TCAM_V4L2_EXPOSURE_TIME_US },
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO,
        .v4l2_id = { V4L2_CID_AUTO_EXPOSURE_BIAS,
                     0x009a0901, // exposure-auto
                     0x0199e202 // auto-shutter
        },
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_REFERENCE,
        .v4l2_id = { 0x0199e203},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT_AUTO,
        .v4l2_id = {0x199e254},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT,
        .v4l2_id = {0x199e256},
    },
    {
        .id = TCAM_PROPERTY_EXPOSURE_AUTO_LOWER_LIMIT,
        .v4l2_id = {0x199e255},
    },
    {
        .id = TCAM_PROPERTY_OVERRIDE_SCANNING_MODE,
        .v4l2_id = { 0x199e257 },
    },
    {
        .id = TCAM_PROPERTY_GAIN,
        .v4l2_id = { V4L2_CID_GAIN },
    },
    {
        .id = TCAM_PROPERTY_GAIN_RED,
        .v4l2_id = { 0x980921, TCAM_V4L2_CID_EUVC_GAIN_R },
    },
    {
        .id = TCAM_PROPERTY_GAIN_GREEN,
        .v4l2_id = {  0x980922, TCAM_V4L2_CID_EUVC_GAIN_G },
    },
    {
        .id = TCAM_PROPERTY_GAIN_BLUE,
        .v4l2_id = {  0x980923, TCAM_V4L2_CID_EUVC_GAIN_B },
    },
    {
        .id = TCAM_PROPERTY_GAIN_AUTO,
        .v4l2_id = { 0x0199e205 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_MODE,
        .v4l2_id = { V4L2_CID_PRIVACY, 0x0199e208, 0x980924},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_SOURCE,
        .v4l2_id = {0},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_ACTIVATION,
        .v4l2_id = {0},
    },
    {
        .id = TCAM_PROPERTY_SOFTWARETRIGGER,
        .v4l2_id = {/* usb 2: */ 0x980926, /* usb 3: */ 0x0199e209},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_DELAY,
        .v4l2_id = {0x199e210},
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_POLARITY,
        .v4l2_id = { 0x0199e234 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_OPERATION,
        .v4l2_id = { 0x0199e235 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_EXPOSURE_MODE,
        .v4l2_id = { 0x0199e236 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_BURST_COUNT,
        .v4l2_id = { 0x0199e237 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_DEBOUNCE_TIME_US,
        .v4l2_id = { 0x0199e238 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_MASK_TIME_US,
        .v4l2_id = { 0x0199e239 },
    },
    {
        .id = TCAM_PROPERTY_TRIGGER_NOISE_SURPRESSION_TIME_US,
        .v4l2_id = { 0x0199e240 },
    },


    {
        .id = TCAM_PROPERTY_WB_PRESET,
        .v4l2_id = {/* usb 3: */ 0x0199e207},
    },
    {
        .id = TCAM_PROPERTY_GPIO,
        .v4l2_id = {/* usb 2: */ 0x980920, /* usb 3: */ 0x0199e217},
    },
    {
        .id = TCAM_PROPERTY_GPIN,
        .v4l2_id = {0},
    },
    {
        .id = TCAM_PROPERTY_GPOUT,
        .v4l2_id = {0x0199e216},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_X,
        .v4l2_id = {0x00980927 /*usb2*/, 0x0199e218 /*usb3*/},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_Y,
        .v4l2_id = {0x00980928 /*usb2*/, 0x0199e219/*usb3*/},
    },
    {
        .id = TCAM_PROPERTY_OFFSET_AUTO,
        .v4l2_id = {0x0199e220},
    },
    {
        .id = TCAM_PROPERTY_BRIGHTNESS,
        .v4l2_id = {V4L2_CID_BRIGHTNESS},
    },
    {
        .id = TCAM_PROPERTY_CONTRAST,
        .v4l2_id = {V4L2_CID_CONTRAST},
    },
    {
        .id = TCAM_PROPERTY_SATURATION,
        .v4l2_id = {V4L2_CID_SATURATION},
    },
    {
        .id = TCAM_PROPERTY_HUE,
        .v4l2_id = {V4L2_CID_HUE},
    },
    {
        .id = TCAM_PROPERTY_GAMMA,
        .v4l2_id = {V4L2_CID_GAMMA},
    },
    {
        .id = TCAM_PROPERTY_WB,
        .v4l2_id = { 0x199e246 },
    },
    {
        .id = TCAM_PROPERTY_WB_AUTO,
        .v4l2_id = {0x0098090c},
    },
    {
        .id = TCAM_PROPERTY_WB_RED,
        .v4l2_id = {0x0098090e},
    },
    {
        .id = TCAM_PROPERTY_WB_GREEN,
        .v4l2_id = { 0x199e248 },
    },
    {
        .id = TCAM_PROPERTY_WB_BLUE,
        .v4l2_id = {0x0098090f},
    },
    {
        .id = TCAM_PROPERTY_WB_AUTO_PRESET,
        .v4l2_id = { 0x199e247 },
    },
    {
        .id = TCAM_PROPERTY_WB_TEMPERATURE,
        .v4l2_id = { 0x199e250 },
    },
    {
        .id = TCAM_PROPERTY_BALANCE_WHITE_TEMPERATURE_PRESET,
        .v4l2_id = { 0x199e249 },
    },
    {
        .id = TCAM_PROPERTY_IRCUT,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_IRIS,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_FOCUS,
        .v4l2_id = {V4L2_CID_FOCUS_ABSOLUTE},
    },
    {
        .id = TCAM_PROPERTY_ZOOM,
        .v4l2_id = {V4L2_CID_ZOOM_ABSOLUTE},
    },
    {
        .id = TCAM_PROPERTY_FOCUS_AUTO,
        .v4l2_id = {},
    },
    {
        .id = TCAM_PROPERTY_FOCUS_ONE_PUSH,
        .v4l2_id = {0x199e206},
    },
    {
        .id = TCAM_PROPERTY_STROBE_ENABLE,
        .v4l2_id = {0x0199e211},
    },
    {
        .id = TCAM_PROPERTY_STROBE_DELAY,
        .v4l2_id = {0x199e215},
    },
    {
        .id = TCAM_PROPERTY_STROBE_POLARITY,
        .v4l2_id = {0x199e212},
    },
    {
        .id = TCAM_PROPERTY_STROBE_EXPOSURE,
        .v4l2_id = {0x199e213},
    },
    {
        .id = TCAM_PROPERTY_STROBE_DURATION,
        .v4l2_id = {0x199e214},
    },
    {
        .id = TCAM_PROPERTY_BINNING,
        .v4l2_id = { 0x980925, TCAM_V4L2_CID_BINNING },
    },
    {
        .id = TCAM_PROPERTY_SKIPPING,
        .v4l2_id = { 0x980929, TCAM_V4L2_CID_SKIP },
    },
    {
        .id = TCAM_PROPERTY_SHARPNESS,
        .v4l2_id = { 0x0098091b},
    },
    {
        .id = TCAM_PROPERTY_NOISE_REDUCTION,
        .v4l2_id = { 0x199e232 },
    },
    {
        .id = TCAM_PROPERTY_FACE_DETECTION,
        .v4l2_id = { 0x199e233},
    },
    {
        .id = TCAM_PROPERTY_IMAGE_STABILIZATION,
        .v4l2_id = { 0x199e231 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL,
        .v4l2_id = { 0x199e241 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_LEFT,
        .v4l2_id = { 0x199e242 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_TOP,
        .v4l2_id = { 0x199e243 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_WIDTH,
        .v4l2_id = { 0x199e244 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_HEIGHT,
        .v4l2_id = { 0x199e245 },
    },
    {
        .id = TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_PRESET,
        .v4l2_id = { 0x199e258 },
    },
    {
        .id = TCAM_PROPERTY_REVERSE_X,
        .v4l2_id = { 0x199e226, 0x199e251 },
    },
    {
        .id = TCAM_PROPERTY_REVERSE_Y,
        .v4l2_id = { 0x199e227, 0x199e252 },
    },
    {
        .id = TCAM_PROPERTY_HIGHLIGHT_REDUCTION,
        .v4l2_id = { 0x199e253 },
    },



};

#endif /* TCAM_V4L2_PROPERTY_MAPPING_H */
