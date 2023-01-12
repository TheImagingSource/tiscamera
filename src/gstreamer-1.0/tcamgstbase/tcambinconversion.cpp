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



static const GEnumValue _tcam_bin_conversion_element_values[] =
{
    { TCAM_BIN_CONVERSION_AUTO, "TCAM_BIN_CONVERSION_AUTO", "auto"},
    { TCAM_BIN_CONVERSION_CONVERT, "TCAM_BIN_CONVERSION_CONVERT", "tcamconvert"},
    { TCAM_BIN_CONVERSION_DUTILS, "TCAM_BIN_CONVERSION_DUTILS", "tcamdutils"},
    { TCAM_BIN_CONVERSION_CUDA, "TCAM_BIN_CONVERSION_CUDA", "tcamdutils-cuda"},
    { 0, nullptr, nullptr}
};

GType tcam_bin_conversion_element_get_type(void)
{
    static GType type = 0;
    if (!type)
    {
        type = g_enum_register_static("TcamBinConversionElement", _tcam_bin_conversion_element_values);
    }
    return type;
}



namespace
{


GstCaps* filter_by_caps_properties(const GstCaps* input, const GstCaps* filter)
{

    GstStructure* filter_struc = gst_caps_get_structure(filter, 0);

    GstStructure* input_struc = gst_caps_get_structure(input, 0);
    if (g_strcmp0(gst_structure_get_name(input_struc), "image/jpeg") == 0)
    {
        return gst_caps_copy(input);
    }

    GstCaps* internal_filter = gst_caps_new_empty();

    for (guint i = 0; i < gst_caps_get_size(input); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(input, i);

        if (gst_structure_get_field_type(struc, "format") == G_TYPE_STRING)
        {
            //const char* string = gst_structure_get_string(struc, "format");

            GstStructure* new_struc = gst_structure_new(gst_structure_get_name(struc),
                                                        "format",
                                                        G_TYPE_STRING,
                                                        gst_structure_get_string(struc, "format"),
                                                        nullptr);

            for (gint x = 0; x < gst_structure_n_fields(filter_struc); x++)
            {
                if (g_strcmp0("format", gst_structure_nth_field_name(filter_struc, x)) == 0)
                {
                    continue;
                }

                gst_structure_set_value(new_struc,
                                        gst_structure_nth_field_name(filter_struc, x),
                                        gst_structure_get_value(filter_struc, gst_structure_nth_field_name(filter_struc, x)));
            }
            gst_caps_append_structure(internal_filter, new_struc);

        }
    }

    internal_filter = gst_caps_simplify(internal_filter);


    GstCaps* ret = gst_caps_intersect((GstCaps*)input, (GstCaps*)internal_filter);

    gst_caps_unref(internal_filter);

    return ret;

}




struct conversion_desc
{
    CAPS_TYPE caps_type_in;
    CAPS_TYPE caps_type_out;
    struct input_caps_required_modules modules;
};

// order of required_modules fields

// bool tcamconvert = false;
// bool videoconvert = false;
// bool jpegdec = false;
// bool dutils = false;

// currently the following pipelines are possible

// tcamsrc
// tcamsrc ! tcamconvert
// tcamsrc ! tcamconvert ! videoconvert
// tcamsrc ! jpegdec
// tcamsrc ! videoconvert
// tcamsrc ! tcamdutils

static const struct conversion_desc tcambin_conversion [] =
{
    // non dutils conversions

    {CAPS_TYPE::BAYER_8, CAPS_TYPE::BAYER_8, {true, false, false, false},},
    {CAPS_TYPE::BAYER_8, CAPS_TYPE::RGB_32, {true, false, false, false},},

    {CAPS_TYPE::BAYER_10, CAPS_TYPE::RGB_32, {true, false, false, false},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_8, {true, false, false, false},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_16, {true, false, false, false},},

    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_16, {true, false, false, false},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_8, {true, false, false, false},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::RGB_32, {true, false, false, false},},

    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_16, {true, false, false, false},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_8, {true, false, false, false},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::RGB_32, {true, false, false, false},},

    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_8, {true, false, false, false},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_16, {true, false, false, false},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::RGB_32, {true, false, false, false},},

    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_8, {true, false, false, false},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_16, {true, false, false, false},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::RGB_32, {true, false, false, false},},

    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_8, {true, false, false, false},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_16, {true, false, false, false},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::RGB_32, {true, false, false, false},},

    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_8, {true, false, false, false},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_16, {true, false, false, false},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::RGB_32, {true, false, false, false},},

    {CAPS_TYPE::JPEG, CAPS_TYPE::JPEG, {false, false, false, false},},
    {CAPS_TYPE::JPEG, CAPS_TYPE::RGB_32, {false, false, true, false},},

    // {CAPS_TYPE::RGB_24, CAPS_TYPE::RGB_24, {false, false, false, false},},
    // {CAPS_TYPE::RGB_24, CAPS_TYPE::RGB_32, {false, true, false, false},},
    // {CAPS_TYPE::YUV, CAPS_TYPE::RGB_32, {false, true, false, false},},

    // the following are dutils specific

    {CAPS_TYPE::TIS_POLARIZED, CAPS_TYPE::RGB_32, {false, false, false, true},},
    {CAPS_TYPE::TIS_POLARIZED, CAPS_TYPE::TIS_POLARIZED, {false, false, false, true},},
    {CAPS_TYPE::FLOATING, CAPS_TYPE::FLOATING, {false, false, false, true},},
    {CAPS_TYPE::FLOATING, CAPS_TYPE::RGB_32, {false, false, false, true},},

    {CAPS_TYPE::BAYER_8, CAPS_TYPE::BAYER_8, {false, false, false, true},},
    {CAPS_TYPE::BAYER_8, CAPS_TYPE::RGB_32, {false, false, false, true},},

    {CAPS_TYPE::BAYER_10, CAPS_TYPE::RGB_32, {false, false, false, true},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::RGB_64, {false, false, false, true},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_8, {false, false, false, true},},
    {CAPS_TYPE::BAYER_10, CAPS_TYPE::BAYER_16, {false, false, false, true},},

    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_16, {false, false, false, true},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_8, {false, false, false, true},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::RGB_32, {false, false, false, true},},
    {CAPS_TYPE::BAYER_12, CAPS_TYPE::RGB_64, {false, false, false, true},},

    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_16, {false, false, false, true},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::BAYER_8, {false, false, false, true},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::RGB_32, {false, false, false, true},},
    {CAPS_TYPE::BAYER_16, CAPS_TYPE::RGB_64, {false, false, false, true},},

    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_8, {false, false, false, true},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::MONO_16, {false, false, false, true},},
    {CAPS_TYPE::MONO_8, CAPS_TYPE::RGB_32, {false, false, false, true},},

    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_8, {false, false, false, true},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_16, {false, false, false, true},},
    {CAPS_TYPE::MONO_10, CAPS_TYPE::RGB_32, {false, false, false, true},},

    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_8, {false, false, false, true},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::MONO_16, {false, false, false, true},},
    {CAPS_TYPE::MONO_12, CAPS_TYPE::RGB_32, {false, false, false, true},},

    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_8, {false, false, false, true},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::MONO_16, {false, false, false, true},},
    {CAPS_TYPE::MONO_16, CAPS_TYPE::RGB_32, {false, false, false, true},},

    {CAPS_TYPE::BAYER_PWL, CAPS_TYPE::RGB_32, {false, false, false, true},},
};


static const CAPS_TYPE ALL_CAPS_TYPES[] = {
    CAPS_TYPE::BAYER_8, CAPS_TYPE::BAYER_10,      CAPS_TYPE::BAYER_12, CAPS_TYPE::BAYER_16,
    CAPS_TYPE::RGB_24,  CAPS_TYPE::RGB_32,        CAPS_TYPE::RGB_64,   CAPS_TYPE::MONO_8,
    CAPS_TYPE::MONO_10, CAPS_TYPE::MONO_12,       CAPS_TYPE::MONO_16,  CAPS_TYPE::JPEG,
    CAPS_TYPE::YUV,     CAPS_TYPE::TIS_POLARIZED, CAPS_TYPE::FLOATING, CAPS_TYPE::BAYER_PWL,
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
        case CAPS_TYPE::BAYER_PWL:
        {
            return gst_caps_from_string("video/x-bayer,format={pwl-rggb12,pwl-rggb12m,pwl-rggb16H12}");
        }
        default:
        {
            return nullptr;
        }
    }
}

} // namespace


namespace tcam::gst
{


GstCaps* find_input_caps(GstCaps* available_caps,
                         GstCaps* wanted_caps,
                         input_caps_required_modules& modules,
                         TcamBinConversionElement toggles)
{
    modules = {};

    if (!GST_IS_CAPS(available_caps))
    {
        return nullptr;
    }

    if (wanted_caps == nullptr || gst_caps_is_empty(wanted_caps))
    {
        GST_INFO("No sink caps specified. Continuing with output caps identical to device caps.");
        wanted_caps = gst_caps_copy(available_caps);
    }

    TcamBinConversion conversion;

    // the negotiation is not as obvious as one would hope
    //
    // available_caps at this point in time might already be reduced in scope
    // if tcambin->device-caps are set they will be available caps
    // filter_by_caps_properties reduces available caps by the settings in wanted_caps
    //
    // This reduction is to ensure that tcam_gst_find_largest_caps behaves correctly
    // Example:
    // camera has maximum of 4000x3000 @ 60fps
    // wanted_caps: video/x-raw,format=BGRx,framerate=15/1
    // find_largest_caps would return 4000x3000 @ 60fps
    // we actually want 4000x3000 @ 15fps
    // thus already remove all caps that do not have 15fps
    // to ensure that is the only this largest caps can find.
    //
    // toggles is an overwrite for the module selection
    // this e.g. allows disabling tcamdutils even if they are found
    // get_modules dictates the modules tcambin shall use, dependent on caps and toggles


    GstCaps* actual_input;

    // tcambin device-caps have been used
    if (gst_caps_is_fixed(available_caps))
    {
        actual_input = gst_caps_copy(available_caps);
    }
    else
    {
        GstCaps* used_caps = filter_by_caps_properties(available_caps, wanted_caps);

        actual_input = tcam_gst_find_largest_caps(used_caps, wanted_caps);

        gst_caps_unref(used_caps);
    }
    modules = conversion.get_modules(actual_input, wanted_caps, toggles);

    return actual_input;
}


TcamBinConversion::TcamBinConversion()
{
    m_caps_table.reserve(std::size(ALL_CAPS_TYPES));
    for (const CAPS_TYPE t : ALL_CAPS_TYPES)
    {
        auto ptr = gst_helper::make_ptr(get_caps_type_definition(t));
        m_caps_table.push_back({ t, ptr });
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
                                                                  TcamBinConversionElement toggles) const
{

    if (!caps || !gst_caps_is_fixed(caps))
    {
        return {};
    }

    std::vector<input_caps_required_modules> collection;

    for (const auto& conv : tcambin_conversion)
    {
        // be compatible to input/output caps
        // collect all potential matches
        // if flags say we should use dutils
        // but a conversion with them is not possible
        // we still want to find them
        if (is_compatible(caps, conv.caps_type_in)
            && is_compatible(wanted_output, conv.caps_type_out))
        {
            collection.push_back(conv.modules);
        }
    }

    if (collection.empty())
    {
        return {};
    }

    if (collection.size() == 1)
    {
        return collection.at(0);
    }

    for (const auto& c : collection)
    {

        if (c.dutils && toggles == TCAM_BIN_CONVERSION_DUTILS)
        {
            return c;
        }
        else if (c.dutils && toggles == TCAM_BIN_CONVERSION_CONVERT)
        {
            return c;
        }
    }

    return {};
}


} // namespace tcam::gst
