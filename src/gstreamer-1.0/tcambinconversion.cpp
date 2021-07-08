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

#include "../logging.h"

using namespace tcam::gst;

namespace {


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



}


namespace tcam::gst
{

TcamBinConversion::TcamBinConversion()
{
    m_caps_table.reserve(std::size(ALL_CAPS_TYPES));
    for (const CAPS_TYPE t : ALL_CAPS_TYPES)
    {
        m_caps_table.push_back({t, get_caps_type_definition(t)});
    }
}

TcamBinConversion::~TcamBinConversion()
{
    for (const auto& t : m_caps_table)
    {
        gst_caps_unref(t.caps);
    }
}

GstCaps* TcamBinConversion::get_caps(CAPS_TYPE type) const
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
    GstCaps* comp_caps = get_caps(compatible_with);

    if (!comp_caps)
    {
        SPDLOG_ERROR("No caps description for {}", compatible_with);
        return false;
    }

    bool ret = gst_caps_can_intersect(to_check, comp_caps);

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
