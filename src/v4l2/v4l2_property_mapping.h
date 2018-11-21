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
        TCAM_PROPERTY_INVALID,
        {},
    },
    {
        TCAM_PROPERTY_EXPOSURE,
        { V4L2_CID_EXPOSURE_ABSOLUTE, V4L2_CID_EXPOSURE,
        // TCAM_V4L2_EXPOSURE_TIME_US
        },
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO,
        { V4L2_CID_AUTO_EXPOSURE_BIAS,
                     0x009a0901, // exposure-auto
                     0x0199e202 // auto-shutter
        },
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_REFERENCE,
        { 0x0199e203},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT_AUTO,
        {0x199e254},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT,
        {0x199e256},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_LOWER_LIMIT,
        {0x199e255},
    },
    {
        TCAM_PROPERTY_GAIN,
        { V4L2_CID_GAIN },
    },
    {
        TCAM_PROPERTY_GAIN_RED,
        { 0x980921, TCAM_V4L2_CID_EUVC_GAIN_R },
    },
    {
        TCAM_PROPERTY_GAIN_GREEN,
        {  0x980922, TCAM_V4L2_CID_EUVC_GAIN_G },
    },
    {
        TCAM_PROPERTY_GAIN_BLUE,
        {  0x980923, TCAM_V4L2_CID_EUVC_GAIN_B },
    },
    {
        TCAM_PROPERTY_GAIN_AUTO,
        { 0x0199e205 },
    },
    {
        TCAM_PROPERTY_TRIGGER_MODE,
        { V4L2_CID_PRIVACY, 0x0199e208, 0x980924},
    },
    {
        TCAM_PROPERTY_TRIGGER_SOURCE,
        {0},
    },
    {
        TCAM_PROPERTY_TRIGGER_ACTIVATION,
        {0},
    },
    {
        TCAM_PROPERTY_SOFTWARETRIGGER,
        {/* usb 2: */ 0x980926, /* usb 3: */ 0x0199e209},
    },
    {
        TCAM_PROPERTY_TRIGGER_DELAY,
        {0x199e210},
    },
    {
        TCAM_PROPERTY_TRIGGER_POLARITY,
        { 0x0199e234 },
    },
    {
        TCAM_PROPERTY_TRIGGER_OPERATION,
        { 0x0199e235 },
    },
    {
        TCAM_PROPERTY_TRIGGER_EXPOSURE_MODE,
        { 0x0199e236 },
    },
    {
        TCAM_PROPERTY_TRIGGER_BURST_COUNT,
        { 0x0199e237 },
    },
    {
        TCAM_PROPERTY_TRIGGER_DEBOUNCE_TIME_US,
        { 0x0199e238 },
    },
    {
        TCAM_PROPERTY_TRIGGER_MASK_TIME_US,
        { 0x0199e239 },
    },
    {
        TCAM_PROPERTY_TRIGGER_NOISE_SURPRESSION_TIME_US,
        { 0x0199e240 },
    },


    {
        TCAM_PROPERTY_WB_PRESET,
        {/* usb 3: */ 0x0199e207},
    },
    {
        TCAM_PROPERTY_GPIO,
        {/* usb 2: */ 0x980920, /* usb 3: */ 0x0199e217},
    },
    {
        TCAM_PROPERTY_GPIN,
        {0},
    },
    {
        TCAM_PROPERTY_GPOUT,
        {0x0199e216},
    },
    {
        TCAM_PROPERTY_OFFSET_X,
        {0x00980927 /*usb2*/, 0x0199e218 /*usb3*/},
    },
    {
        TCAM_PROPERTY_OFFSET_Y,
        {0x00980928 /*usb2*/, 0x0199e219/*usb3*/},
    },
    {
        TCAM_PROPERTY_OFFSET_AUTO,
        {0x0199e220},
    },
    {
        TCAM_PROPERTY_BRIGHTNESS,
        {V4L2_CID_BRIGHTNESS},
    },
    {
        TCAM_PROPERTY_CONTRAST,
        {V4L2_CID_CONTRAST},
    },
    {
        TCAM_PROPERTY_SATURATION,
        {V4L2_CID_SATURATION},
    },
    {
        TCAM_PROPERTY_HUE,
        {V4L2_CID_HUE},
    },
    {
        TCAM_PROPERTY_GAMMA,
        {V4L2_CID_GAMMA},
    },
    {
        TCAM_PROPERTY_WB,
        { 0x199e246 },
    },
    {
        TCAM_PROPERTY_WB_AUTO,
        {0x0098090c},
    },
    {
        TCAM_PROPERTY_WB_RED,
        {0x0098090e},
    },
    {
        TCAM_PROPERTY_WB_GREEN,
        { 0x199e248 },
    },
    {
        TCAM_PROPERTY_WB_BLUE,
        {0x0098090f},
    },
    {
        TCAM_PROPERTY_WB_AUTO_PRESET,
        { 0x199e247 },
    },
    {
        TCAM_PROPERTY_WB_TEMPERATURE,
        { 0x199e250 },
    },
    {
        TCAM_PROPERTY_BALANCE_WHITE_TEMPERATURE_PRESET,
        { 0x199e249 },
    },
    {
        TCAM_PROPERTY_IRCUT,
        {},
    },
    {
        TCAM_PROPERTY_IRIS,
        {},
    },
    {
        TCAM_PROPERTY_FOCUS,
        {V4L2_CID_FOCUS_ABSOLUTE},
    },
    {
        TCAM_PROPERTY_ZOOM,
        {V4L2_CID_ZOOM_ABSOLUTE},
    },
    {
        TCAM_PROPERTY_FOCUS_AUTO,
        {},
    },
    {
        TCAM_PROPERTY_FOCUS_ONE_PUSH,
        {0x199e206},
    },
    {
        TCAM_PROPERTY_STROBE_ENABLE,
        {0x0199e211},
    },
    {
        TCAM_PROPERTY_STROBE_DELAY,
        {0x199e215},
    },
    {
        TCAM_PROPERTY_STROBE_POLARITY,
        {0x199e212},
    },
    {
        TCAM_PROPERTY_STROBE_EXPOSURE,
        {0x199e213},
    },
    {
        TCAM_PROPERTY_STROBE_DURATION,
        {0x199e214},
    },
    {
        TCAM_PROPERTY_BINNING,
        { 0x980925, TCAM_V4L2_CID_BINNING },
    },
    {
        TCAM_PROPERTY_SKIPPING,
        { 0x980929, TCAM_V4L2_CID_SKIP },
    },
    {
        TCAM_PROPERTY_SHARPNESS,
        { 0x0098091b},
    },
    {
        TCAM_PROPERTY_NOISE_REDUCTION,
        { 0x199e232 },
    },
    {
        TCAM_PROPERTY_FACE_DETECTION,
        { 0x199e233},
    },
    {
        TCAM_PROPERTY_IMAGE_STABILIZATION,
        { 0x199e231 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL,
        { 0x199e241 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_LEFT,
        { 0x199e242 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_TOP,
        { 0x199e243 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_WIDTH,
        { 0x199e244 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_HEIGHT,
        { 0x199e245 },
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_PRESET,
        { 0x199e258 },
    },
    {
        TCAM_PROPERTY_REVERSE_X,
        { 0x199e226, 0x199e251 },
    },
    {
        TCAM_PROPERTY_REVERSE_Y,
        { 0x199e227, 0x199e252 },
    },
    {
        TCAM_PROPERTY_HIGHLIGHT_REDUCTION,
        { 0x199e253 },
    },



};

#endif /* TCAM_V4L2_PROPERTY_MAPPING_H */
