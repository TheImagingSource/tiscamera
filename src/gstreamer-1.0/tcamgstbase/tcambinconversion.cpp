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

#include "tcambinconversion.h"

#include "../../logging.h"

using namespace tcam::gst;

namespace
{

struct conversion_desc
{
    CAPS_TYPE caps_type_in;
    CAPS_TYPE caps_type_out;
    struct input_caps_required_modules modules;
};

// order of required_modules fields
// bool tcamconvert = false;
// bool bayer2rgb = false;
// bool videoconvert = false;
// bool jpegdec = false;
// bool dutils = false;

static const struct conversion_desc tcambin_conversion [] =
{
    // non dutils conversions

    {CAPS_TYPE::BAYER_8, CAPS_TYPE::BAYER_8, {true, false, false, false, false},},
    {CAPS_TYPE::BAYER_8, CAPS_TYPE::RGB_32, {true, true, false, false, false},},

    {CAPS_TYPE::BAYER_10, CAPS_TYPE::RGB_32, {true, true, false, false, false},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_8, {true, false, false, false, false},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_16, {true, false, false, false, false},},

    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_16, {true, false, false, false, false},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_8, {true, false, false, false, false},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::RGB_32, {true, true, false, false, false},},

    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_16, {true, false, false, false, false},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_8, {true, false, false, false, false},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::RGB_32, {true, true, false, false, false},},

    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_8, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_16, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::RGB_32, {true, false, true, false, false},},

    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_8, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_16, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::RGB_32, {true, false, true, false, false},},

    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_8, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_16, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::RGB_32, {true, false, true, false, false},},

    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_8, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_16, {true, false, false, false, false},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::RGB_32, {true, false, true, false, false},},

    {CAPS_TYPE::JPEG, CAPS_TYPE::JPEG, {false, false, false, false, false},},
    {CAPS_TYPE::JPEG, CAPS_TYPE::RGB_32, {false, false, false, true, false},},

    {CAPS_TYPE::RGB_24, CAPS_TYPE::RGB_24, {false, false, false, false, false},},
    {CAPS_TYPE::RGB_24, CAPS_TYPE::RGB_32, {false, false, true, false, false},},
    {CAPS_TYPE::YUV, CAPS_TYPE::RGB_32, {false, false, true, false, false},},

    // the following are dutils specific

    {CAPS_TYPE::TIS_POLARIZED, CAPS_TYPE::RGB_32, {false, false, false, false, true},},
    {CAPS_TYPE::TIS_POLARIZED, CAPS_TYPE::TIS_POLARIZED, {false, false, false, false, true},},
    {CAPS_TYPE::FLOATING, CAPS_TYPE::FLOATING, {false, false, false, false, true},},
    {CAPS_TYPE::FLOATING, CAPS_TYPE::RGB_32, {false, false, false, false, true},},

    {CAPS_TYPE::BAYER_8, CAPS_TYPE::BAYER_8, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_8, CAPS_TYPE::RGB_32, {false, false, false, false, true},},

    {CAPS_TYPE::BAYER_10, CAPS_TYPE::RGB_32, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::RGB_64, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_8, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_16, {false, false, false, false, true},},

    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_16, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_8, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::RGB_32, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::RGB_64, {false, false, false, false, true},},

    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_16, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_8, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::RGB_32, {false, false, false, false, true},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::RGB_64, {false, false, false, false, true},},

    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_8, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_16, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::RGB_32, {false, false, false, false, true},},

    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_8, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_16, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::RGB_32, {false, false, false, false, true},},

    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_8, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_16, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::RGB_32, {false, false, false, false, true},},

    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_8, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_16, {false, false, false, false, true},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::RGB_32, {false, false, false, false, true},},
};


static const CAPS_TYPE ALL_CAPS_TYPES[] = {
    CAPS_TYPE::BAYER_8, CAPS_TYPE::BAYER_10,      CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_16,
    CAPS_TYPE::RGB_24,  CAPS_TYPE::RGB_32,        CAPS_TYPE::RGB_64,   CAPS_TYPE::MONO_8,
    CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_12,       CAPS_TYPE::MONO_16,  CAPS_TYPE::JPEG,
    CAPS_TYPE::YUV,     CAPS_TYPE::TIS_POLARIZED, CAPS_TYPE::FLOATING,
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

}


namespace tcam::gst
{

TcamBinConversion::TcamBinConversion()
{
    m_caps_table.reserve(std::size(ALL_CAPS_TYPES));
    for (const CAPS_TYPE t : ALL_CAPS_TYPES)
    {
        auto ptr = gst_helper::make_ptr( get_caps_type_definition( t ) );
        m_caps_table.push_back({t, ptr});
    }
}

gst_helper::gst_ptr<GstCaps> TcamBinConversion::get_caps(CAPS_TYPE type) const
{
    for (const auto& m : m_caps_table)
    {
        if (m.type == type)
        {
            return m.caps;
        }
    }
    return nullptr;
}

bool TcamBinConversion::is_compatible(GstCaps* to_check, CAPS_TYPE compatible_with) const
{
    auto comp_caps = get_caps(compatible_with);

    if (!comp_caps)
    {
        SPDLOG_ERROR("No caps description for {}", compatible_with);
        return false;
    }

    bool ret = gst_caps_can_intersect(to_check, comp_caps.get());

    return ret;
}

struct input_caps_required_modules TcamBinConversion::get_modules(GstCaps* caps,
                                                                  GstCaps* wanted_output,
                                                                  struct input_caps_toggles toggles) const
{

    if (!gst_caps_is_fixed(caps))
    {
        return {};
    }

    for (const auto& conv : tcambin_conversion)
    {
        // be compatible to input/output caps
        // also ensure we have a a compatible conversion
        // dutils and non-dutils conversions are in the same table
        if (is_compatible(caps, conv.caps_type_in)
            && is_compatible(wanted_output, conv.caps_type_out)
            && toggles.use_dutils == conv.modules.dutils)
        {
            return conv.modules;
        }
    }
    return {};
}


}
