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

#ifndef TCAM_GENICAM_PROPERTY_MAPPING_H
#define TCAM_GENICAM_PROPERTY_MAPPING_H

#include "standard_properties.h"

#include <vector>
#include <algorithm>
#include <string>

using namespace tcam;


struct aravis_property
{
    TCAM_PROPERTY_ID id;
    std::vector<std::string> genicam_name; // list of genicam identifiers that shall be mapped to control
};


static std::vector<struct aravis_property> aravis_mappings =
{
    {
        TCAM_PROPERTY_INVALID,
        {},
    },
    {
        TCAM_PROPERTY_EXPOSURE,
        {"ExposureTime"},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO,
        {"ExposureAuto"},
    },
    {
        TCAM_PROPERTY_HIGHLIGHT_REDUCTION,
        {"ExposureAutoHighlighReduction"},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_REFERENCE,
        {"ExposureAutoReference"},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT_AUTO,
        {"ExposureAutoUpperLimitAuto"},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_UPPER_LIMIT,
        {"ExposureAutoUpperLimit"},
    },
    {
        TCAM_PROPERTY_EXPOSURE_AUTO_LOWER_LIMIT,
        {"ExposureAutoLowerLimit"},
    },
    {
        TCAM_PROPERTY_GAIN,
        {"Gain"},
    },
    {
        TCAM_PROPERTY_GAIN_AUTO,
        {"GainAuto"},
    },
    {
        TCAM_PROPERTY_GAIN_RED,
        {},
    },
    {
        TCAM_PROPERTY_GAIN_GREEN,
        {},
    },
    {
        TCAM_PROPERTY_GAIN_BLUE,
        {},
    },
    {
        TCAM_PROPERTY_GAIN_AUTO_UPPER_LIMIT,
        {"GainAutoUpperLimit"},
    },
    {
        TCAM_PROPERTY_GAIN_AUTO_LOWER_LIMIT,
        {"GainAutoLowerLimit"},
    },
    {
        TCAM_PROPERTY_TRIGGER_MODE,
        {"TriggerMode"},
    },
    {
        TCAM_PROPERTY_TRIGGER_SOURCE,
        {"TriggerSource"},
    },
    {
        TCAM_PROPERTY_TRIGGER_ACTIVATION,
        {"TriggerActivation"},
    },
    {
        TCAM_PROPERTY_SOFTWARETRIGGER,
        {"TriggerSoftware"},
    },
    {
        TCAM_PROPERTY_TRIGGER_DENOISE,
        {"TriggerDenoise"},
    },
    {
        TCAM_PROPERTY_TRIGGER_MASK,
        {"TriggerMask"},
    },
    {
        TCAM_PROPERTY_TRIGGER_DEBOUNCER,
        {"TriggerDebouncer"},
    },
    {
        TCAM_PROPERTY_TRIGGER_DELAY,
        {"TriggerDelay"},
    },
    {
        TCAM_PROPERTY_TRIGGER_SELECTOR,
        {"TriggerSelector"},
    },
    {
        TCAM_PROPERTY_TRIGGER_OPERATION,
        {"TriggerOperation"},
    },
    {
        TCAM_PROPERTY_GPIO,
        {"GPIO"},
    },
    {
        TCAM_PROPERTY_GPIN,
        {"GPIn"},
    },
    {
        TCAM_PROPERTY_GPOUT,
        {"GPOut"},
    },
    {
        TCAM_PROPERTY_OFFSET_X,
        {"OffsetX"},
    },
    {
        TCAM_PROPERTY_OFFSET_Y,
        {"OffsetY"},
    },
    {
        TCAM_PROPERTY_OFFSET_AUTO,
        {"OffsetAutoCenter"},
    },
    {
        TCAM_PROPERTY_BRIGHTNESS,
        {"Brightness"},
    },
    {
        TCAM_PROPERTY_CONTRAST,
        {"Contrast"},
    },
    {
        TCAM_PROPERTY_SATURATION,
        {"Saturation"},
    },
    {
        TCAM_PROPERTY_HUE,
        {"Hue"},
    },
    {
        TCAM_PROPERTY_GAMMA,
        {"Gamma"},
    },
    {
        TCAM_PROPERTY_WB_AUTO,
        {"BalanceWhiteAuto"},
    },
    {
        TCAM_PROPERTY_WB_MODE,
        {"BalanceWhiteMode"},
    },
    {
        TCAM_PROPERTY_WB_TEMPERATURE,
        {"BalanceWhiteTemperature"},
    },
    {
        TCAM_PROPERTY_BALANCERATIO_SELECTOR,
        {"BalanceRatioSelector"}
    },
    {
        TCAM_PROPERTY_BALANCERATIO,
        {"BalanceRatio"}
    },
    {
        TCAM_PROPERTY_BALANCE_WHITE_AUTO_PRESET,
        {"BalanceWhiteAutoPreset"}
    },
    {
        TCAM_PROPERTY_BALANCE_WHITE_TEMPERATURE_PRESET,
        {"BalanceWhiteTemperaturePreset"}
    },
    {
        TCAM_PROPERTY_IRCUT,
        {"IRCutFilter"},
    },
    {
        TCAM_PROPERTY_IRIS,
        {"Iris"},
    },
    {
        TCAM_PROPERTY_FOCUS,
        {"Focus"},
    },
    {
        TCAM_PROPERTY_ZOOM,
        {"Zoom"},
    },
    {
        TCAM_PROPERTY_FOCUS_AUTO,
        {"FocusAuto"},
    },
    {
        TCAM_PROPERTY_STROBE_ENABLE,
        {"StrobeEnable"},
    },
    {
        TCAM_PROPERTY_STROBE_OPERATION,
        {"StrobeOperation"},
    },
    {
        TCAM_PROPERTY_STROBE_POLARITY,
        {"StrobePolarity"},
    },
    {
        TCAM_PROPERTY_STROBE_DELAY,
        {"StrobeDelay"},
    },
    {
        TCAM_PROPERTY_STROBE_DURATION,
        {"StrobeDuration"},
    },
    {
        TCAM_PROPERTY_BINNING,
        {"Binning"},
    },
    {
        TCAM_PROPERTY_BINNING_VERTICAL,
        {"BinningHorizontal"},
    },
    {
        TCAM_PROPERTY_BINNING_HORIZONTAL,
        {"BinningVertical"},
    },
    {
        TCAM_PROPERTY_BLACKLEVEL,
        {"BlackLevel"},
    },
    {
        TCAM_PROPERTY_CHUNK_MODE_ACTIVE,
        {"ChunkModeActive"},
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_COUNT,
        {"DeviceStreamChannelCount"},
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_SELECTOR,
        {"DeviceStreamChannelSelector"},
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_TYPE,
        {"DeviceStreamChannelType"},
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_LINK,
        {"DeviceStreamChannelLink"},
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_ENDIANNESS,
        {"DeviceStreamChannelEndianness"},
    },
    {
        TCAM_PROPERTY_STREAM_CHANNEL_PACKET_SIZE,
        {"DeviceStreamChannelPacketSize"},
    },
    {
        TCAM_PROPERTY_EVENT_CHANNEL_COUNT,
        {"DeviceStreamChannelPacketSize"},
    },
    {
        TCAM_PROPERTY_PAYLOAD_SIZE,
        {"PayloadSize"},
    },
    {
        TCAM_PROPERTY_PAYLOAD_PER_FRAME,
        {"PayloadPerFrame"},
    },
    {
        TCAM_PROPERTY_PAYLOAD_PER_PACKET,
        {"PayloadPerPacket"},
    },
    {
        TCAM_PROPERTY_TOTAL_PACKET_SIZE,
        {"TotalPacketSize"},
    },
    {
        TCAM_PROPERTY_PACKETS_PER_FRAME,
        {"PacketsPerFrame"},
    },
    {
        TCAM_PROPERTY_PACKET_TIME_US,
        {"PacketTimeUS"},
    },
    {
        TCAM_PROPERTY_REVERSE_X,
        {"ReverseX"},
    },
    {
        TCAM_PROPERTY_REVERSE_Y,
        {"ReverseY"},
    },
    {
        TCAM_PROPERTY_CHUNK_IMX174_FRAME_ID,
        {"ChunkIMX174FrameId"},
    },
    {
        TCAM_PROPERTY_CHUNK_IMX174_FRAME_SET,
        {"ChunkIMX174FrameSet"},
    },
    {
        TCAM_PROPERTY_IMX174_WDR_SHUTTER2,
        {"IMX174WDRShutter2"},
    },
    {
        TCAM_PROPERTY_IMX174_HARDWARE_WDR_ENABLE,
        {"IMX174HardwareWDREnable"},
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_CONTROL,
        {"AutoFunctionsROIEnable"},
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_LEFT,
        {"AutoFunctionsROILeft"},
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_HEIGHT,
        {"AutoFunctionsROIHeight"},
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_TOP,
        {"AutoFunctionsROITop"},
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_WIDTH,
        {"AutoFunctionsROIWidth"},
    },
    {
        TCAM_PROPERTY_AUTO_FUNCTIONS_ROI_PRESET,
        {"AutoFunctionsROIPreset"},
    },
    {
        TCAM_PROPERTY_TRIGGER_BURST_COUNT,
        {"AcquisitionBurstFrameCount"}
    },
};


static TCAM_PROPERTY_ID find_mapping (const std::string& genicam_id)
{
    auto f = [&genicam_id] (std::string p)
        {
            if (genicam_id == p)
                return true;
            return false;
        };

    for (const auto& m : aravis_mappings)
    {
        auto match = std::find_if(m.genicam_name.begin(), m.genicam_name.end(), f);

        if (match != m.genicam_name.end())
            return m.id;
    }
    return TCAM_PROPERTY_INVALID;
}


#endif /* TCAM_GENICAM_PROPERTY_MAPPING_H */
