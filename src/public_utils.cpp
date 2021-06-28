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

#include "public_utils.h"

#include "utils.h"

#include <algorithm>

using namespace tcam;


std::vector<TCAM_DEVICE_TYPE> tcam::get_device_type_list()
{
    return { TCAM_DEVICE_TYPE_UNKNOWN, TCAM_DEVICE_TYPE_V4L2,   TCAM_DEVICE_TYPE_ARAVIS,
             TCAM_DEVICE_TYPE_LIBUSB,  TCAM_DEVICE_TYPE_PIMIPI, TCAM_DEVICE_TYPE_MIPI,
             TCAM_DEVICE_TYPE_TEGRA };
}


std::vector<std::string> tcam::get_device_type_list_strings()
{
    auto vec = get_device_type_list();

    std::vector<std::string> ret;

    ret.reserve(vec.size());

    for (const auto& v : vec) { ret.push_back(tcam_device_type_to_string(v)); }

    return ret;
}


std::string tcam::tcam_device_type_to_string(TCAM_DEVICE_TYPE type)
{
    switch (type)
    {
        case TCAM_DEVICE_TYPE_V4L2:
            return "v4l2";
        case TCAM_DEVICE_TYPE_ARAVIS:
            return "aravis";
        case TCAM_DEVICE_TYPE_LIBUSB:
            return "libusb";
        case TCAM_DEVICE_TYPE_PIMIPI:
            return "pimipi";
        case TCAM_DEVICE_TYPE_MIPI:
            return "mipi";
        case TCAM_DEVICE_TYPE_TEGRA:
            return "tegra";
        default:
            return "unknown";
    }
}


TCAM_DEVICE_TYPE tcam::tcam_device_from_string(const std::string& input)
{
    std::string str = input;

    std::transform(
        str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });

    if (str == "v4l2")
        return TCAM_DEVICE_TYPE_V4L2;
    else if (str == "aravis")
        return TCAM_DEVICE_TYPE_ARAVIS;
    else if (str == "libusb")
        return TCAM_DEVICE_TYPE_LIBUSB;
    else if (str == "pimipi")
        return TCAM_DEVICE_TYPE_PIMIPI;
    else if (str == "mipi")
        return TCAM_DEVICE_TYPE_MIPI;
    else if (str == "tegra")
        return TCAM_DEVICE_TYPE_TEGRA;

    return TCAM_DEVICE_TYPE_UNKNOWN;
}

std::vector<tcam_image_size> tcam::get_standard_resolutions(const tcam_image_size& min,
                                                            const tcam_image_size& max)
{
    static const tcam_image_size resolutions[] = {
        { 128, 96 },    { 320, 240 },   { 360, 280 },   { 544, 480 },   { 640, 480 },
        { 352, 288 },   { 576, 480 },   { 720, 480 },   { 960, 720 },   { 1280, 720 },
        { 1440, 1080 }, { 1920, 1080 }, { 1920, 1200 }, { 2048, 1152 }, { 2048, 1536 },
        { 2560, 1440 }, { 3840, 2160 }, { 4096, 3072 }, { 7680, 4320 }, { 7680, 4800 },
    };

    std::vector<struct tcam_image_size> ret;
    ret.reserve(std::size_t(resolutions));
    for (const auto& r : resolutions)
    {
        if ((min < r) && (r < max))
        {
            ret.push_back(r);
        }
    }

    return ret;
}
