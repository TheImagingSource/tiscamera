/*
 * Copyright 2021 The Imaging Source Europe GmbH
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

#pragma once

#include <gst/gst.h>

#include <vector>

#include "tcamgstbase.h"

namespace tcam::gst
{


enum class CAPS_TYPE
{
    BAYER_8,
    BAYER_10,
    BAYER_12,
    BAYER_16,
    RGB_24,
    RGB_32,
    RGB_64,
    MONO_8,
    MONO_10,
    MONO_12,
    MONO_16,
    JPEG,
    YUV,
    TIS_POLARIZED,
    FLOATING,
};

static const CAPS_TYPE ALL_CAPS_TYPES [] =
{
    CAPS_TYPE::BAYER_8,
    CAPS_TYPE::BAYER_10,
    CAPS_TYPE::BAYER_12,
    CAPS_TYPE::BAYER_16,
    CAPS_TYPE::RGB_24,
    CAPS_TYPE::RGB_32,
    CAPS_TYPE::RGB_64,
    CAPS_TYPE::MONO_8,
    CAPS_TYPE::MONO_10,
    CAPS_TYPE::MONO_12,
    CAPS_TYPE::MONO_16,
    CAPS_TYPE::JPEG,
    CAPS_TYPE::YUV,
    CAPS_TYPE::TIS_POLARIZED,
    CAPS_TYPE::FLOATING,
};


static GstCaps* get_caps_type_definition(CAPS_TYPE type)
{
    switch (type)
    {
        case CAPS_TYPE::BAYER_8:
        {
            return gst_caps_from_string("video/x-bayer,format={rggb, bggr, gbrg, grbg}");
        }
        case CAPS_TYPE::BAYER_10:
        {
            return gst_caps_from_string("video/x-bayer,format={rggb10, bggr10, gbrg10, grbg10, "
                                        "rggb10p, bggr10p, gbrg10p, grbg10p, "
                                        "rggb10sp, bggr10sp, gbrg10p, grbg10p, "
                                        "rggb10m, bggr10m, gbrg10m, grbg10m}");
        }
        case CAPS_TYPE::BAYER_12:
        {
            return gst_caps_from_string("video/x-bayer,format={rggb12, bggr12, gbrg12, grbg12,"
                                        "rggb12p, bggr12p, gbrg12p, grbg12p, "
                                        "rggb12sp, bggr12sp, gbrg12p, grbg12p, "
                                        "rggb12m, bggr12m, gbrg12m, grbg12m}");
        }
        case CAPS_TYPE::BAYER_16:
        {
            return gst_caps_from_string("video/x-bayer,format={rggb16, bggr16, gbrg16, grbg16}");
        }
        case CAPS_TYPE::RGB_24:
        {
            return gst_caps_from_string("video/x-raw,format=BGR");
        }
        case CAPS_TYPE::RGB_32:
        {
            return gst_caps_from_string("video/x-raw,format=BGRx");
        }
        case CAPS_TYPE::RGB_64:
        {
            return gst_caps_from_string("video/x-raw,format=RGBx64");
        }
        case CAPS_TYPE::MONO_8:
        {
            return gst_caps_from_string("video/x-raw,format=GRAY8");
        }
        case CAPS_TYPE::MONO_10:
        {
            return gst_caps_from_string("video/x-raw,format={GRAY10, GRAY10m, GRAY10sp}");
        }
        case CAPS_TYPE::MONO_12:
        {
            return gst_caps_from_string("video/x-raw,format={GRAY12, GRAY12m, GRAY12sp, GRAY12p}");
        }
        case CAPS_TYPE::MONO_16:
        {
            return gst_caps_from_string("video/x-raw,format=GRAY16_LE");
        }
        case CAPS_TYPE::JPEG:
        {
            return gst_caps_from_string("image/jpeg");
        }
        case CAPS_TYPE::YUV:
        {
            return gst_caps_from_string("video/x-raw,format={YUYV}");
        }
        case CAPS_TYPE::TIS_POLARIZED:
        {
            return gst_caps_from_string("video/x-tis");
        }
        case CAPS_TYPE::FLOATING:
        {
            return gst_caps_from_string("video/x-tis");
        }
        default:
        {
            return nullptr;
        }
    }
}


class TcamBinConversion
{

private:

    struct caps_map
    {
        CAPS_TYPE type;
        GstCaps* caps;

        constexpr caps_map& operator=(const caps_map&) = default;

        ~caps_map()
        {
            if (caps)
            {
                //gst_caps_unref(caps);
            }
        };
    };


    std::vector<struct caps_map> m_caps_table;

public:

    explicit TcamBinConversion();

    ~TcamBinConversion();

    GstCaps* get_caps(CAPS_TYPE type) const;

    bool is_compatible(GstCaps* to_check, CAPS_TYPE compatible_with) const;

    struct input_caps_required_modules get_modules(GstCaps* caps,
                                                   GstCaps* wanted_output,
                                                   struct input_caps_toggles toggles) const;

};

}
