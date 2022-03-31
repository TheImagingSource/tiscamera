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

#include "tcamgstbase.h"

#include "../../base_types.h"
#include "../../logging.h"
#include "tcambinconversion.h"
#include "tcamgststrings.h"

#include <algorithm> //std::find
#include <cstring> // strcmp
#include <dutils_img/fcc_to_string.h>
#include <dutils_img/image_fourcc_func.h>
#include <gst-helper/gst_gvalue_helper.h> // gst_string_list_to_vector
#include <gst-helper/helper_functions.h>
#include <map>


typedef struct tcam_src_element_
{
    std::string name;
    // name so because function `g_type_name` exists
    std::string g_type_name_str;
    std::vector<tcam::TCAM_DEVICE_TYPE> type;
} tcam_src_element;


static std::vector<tcam_src_element> get_possible_sources()
{
    using namespace tcam;

    std::vector<tcam_src_element> ret;

    ret.push_back({ "tcammainsrc",
                    "GstTcamMainSrc",
                    { TCAM_DEVICE_TYPE_V4L2, TCAM_DEVICE_TYPE_ARAVIS, TCAM_DEVICE_TYPE_LIBUSB } });
    ret.push_back({ "tcamtegrasrc", "GstTcamTegraSrc", { TCAM_DEVICE_TYPE_TEGRA } });
    ret.push_back({ "tcampimipisrc", "GstTcamPiMipiSrc", { TCAM_DEVICE_TYPE_PIMIPI } });
    ret.push_back({ "tcamsrc",
                    "GstTcamSrc",
                    { TCAM_DEVICE_TYPE_V4L2,
                      TCAM_DEVICE_TYPE_ARAVIS,
                      TCAM_DEVICE_TYPE_LIBUSB,
                      TCAM_DEVICE_TYPE_TEGRA,
                      TCAM_DEVICE_TYPE_PIMIPI } });

    return ret;
}


static std::vector<std::string> get_source_element_factory_names()
{
    auto sources = get_possible_sources();

    std::vector<std::string> ret;

    ret.reserve(sources.size());

    for (const auto& s : sources) { ret.push_back(s.g_type_name_str); }

    return ret;
}


static GstElement* tcam_gst_find_camera_src_rec(GstElement* element,
                                                const std::vector<std::string>& factory_names)
{
    GstPad* orig_pad = gst_element_get_static_pad(element, "sink");

    GstPad* src_pad = gst_pad_get_peer(orig_pad);
    gst_object_unref(orig_pad);

    if (!src_pad)
    {
        // this means we have reached a dead end where no valid tcamsrc exists
        return nullptr;
    }

    GstElement* el = gst_pad_get_parent_element(src_pad);

    gst_object_unref(src_pad);

    std::string name =
        g_type_name(gst_element_factory_get_element_type(gst_element_get_factory(el)));

    if (std::find(factory_names.begin(), factory_names.end(), name) != factory_names.end())
    {
        return el;
    }

    GstElement* ret = tcam_gst_find_camera_src_rec(el, factory_names);

    gst_object_unref(el);

    return ret;
}


GstElement* tcam::gst::tcam_gst_find_camera_src(GstElement* element)
{
    std::vector<std::string> factory_names = get_source_element_factory_names();

    return tcam_gst_find_camera_src_rec(element, factory_names);
}


std::string tcam::gst::get_plugin_version(const char* plugin_name)
{
    GstPlugin* plugin = gst_plugin_load_by_name(plugin_name);
    if (plugin == nullptr)
    {
        return {};
    }

    std::string rval;
    const char* version_str = gst_plugin_get_version(plugin);
    if (version_str != nullptr)
    {
        rval = version_str;
    }

    gst_object_unref(plugin);

    return rval;
}

static std::vector<std::string> gst_list_to_vector(const GValue* gst_list)
{
    return gst_helper::gst_string_list_to_vector(*gst_list);
}

bool tcam::gst::tcam_gst_raw_only_has_mono(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    auto correct_format = [](const char* str)
    {
        if (str == nullptr)
        {
            return false;
        }

        static const char* formats[] = { "GRAY8",   "GRAY16_LE", "GRAY16_BE", "GRAY12p",
                                         "GRAY10p", "GRAY12m",   "GRAY10m" };

        return std::any_of(std::begin(formats),
                           std::end(formats),
                           [str](const char* op2) { return strcmp(str, op2) == 0; });
    };

    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(caps, i);

        if (strcmp("video/x-raw", gst_structure_get_name(struc)) == 0)
        {
            if (gst_structure_has_field(struc, "format"))
            {
                if (gst_structure_get_field_type(struc, "format") == G_TYPE_STRING)
                {
                    if (!correct_format(gst_structure_get_string(struc, "format")))
                    {
                        return false;
                    }
                }
                else if (gst_structure_get_field_type(struc, "format") == GST_TYPE_LIST)
                {
                    auto vec = gst_list_to_vector(gst_structure_get_value(struc, "format"));

                    for (const auto& fmt : vec)
                    {
                        if (!correct_format(fmt.c_str()))
                        {
                            return false;
                        }
                    }
                }
                else
                {
                    SPDLOG_ERROR("Cannot handle format type in GstStructure.");
                }
            }
            else
            {
                // since raw can be anything
                // do not assume it is gray but color
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}


static bool tcam_gst_is_mono10_fourcc(const uint32_t fourcc)
{
    if (fourcc == FOURCC_MONO10
        || fourcc == FOURCC_MONO10_MIPI_PACKED
        || fourcc == FOURCC_MONO10_SPACKED)
    {
        return true;
    }
    return false;
}


static bool tcam_gst_is_mono12_fourcc(const uint32_t fourcc)
{
    if (fourcc == FOURCC_MONO12
        || fourcc == FOURCC_MONO12_MIPI_PACKED
        || fourcc == FOURCC_MONO12_PACKED
        || fourcc == FOURCC_MONO12_SPACKED)
    {
        return true;
    }
    return false;
}


static bool tcam_gst_is_fourcc_bayer(const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG8 || fourcc == FOURCC_GRBG8 || fourcc == FOURCC_RGGB8
        || fourcc == FOURCC_BGGR8)
    {
        return TRUE;
    }
    return FALSE;
}


static bool tcam_gst_is_bayer10_fourcc(const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG10 || fourcc == FOURCC_GRBG10 || fourcc == FOURCC_RGGB10
        || fourcc == FOURCC_BGGR10)
    {
        return TRUE;
    }
    return FALSE;
}


static bool tcam_gst_is_bayer10_packed_fourcc(const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG10_SPACKED || fourcc == FOURCC_GRBG10_SPACKED
        || fourcc == FOURCC_RGGB10_SPACKED || fourcc == FOURCC_BGGR10_SPACKED
        || fourcc == FOURCC_GBRG10_MIPI_PACKED || fourcc == FOURCC_GRBG10_MIPI_PACKED
        || fourcc == FOURCC_RGGB10_MIPI_PACKED || fourcc == FOURCC_BGGR10_MIPI_PACKED)
    {
        return TRUE;
    }
    return FALSE;
}


static bool tcam_gst_is_bayer12_fourcc(const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG12 || fourcc == FOURCC_GRBG12 || fourcc == FOURCC_RGGB12
        || fourcc == FOURCC_BGGR12)
    {
        return TRUE;
    }
    return FALSE;
}


static bool tcam_gst_is_bayer12_packed_fourcc(const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG12_MIPI_PACKED || fourcc == FOURCC_GRBG12_MIPI_PACKED
        || fourcc == FOURCC_RGGB12_MIPI_PACKED || fourcc == FOURCC_BGGR12_MIPI_PACKED
        || fourcc == FOURCC_GBRG12_SPACKED || fourcc == FOURCC_GRBG12_SPACKED
        || fourcc == FOURCC_RGGB12_SPACKED || fourcc == FOURCC_BGGR12_SPACKED
        || fourcc == FOURCC_GBRG12_PACKED || fourcc == FOURCC_GRBG12_PACKED
        || fourcc == FOURCC_RGGB12_PACKED || fourcc == FOURCC_BGGR12_PACKED)
    {
        return TRUE;
    }
    return FALSE;
}


static bool tcam_gst_is_bayer16_fourcc(const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG16 || fourcc == FOURCC_GRBG16 || fourcc == FOURCC_RGGB16
        || fourcc == FOURCC_BGGR16)
    {
        return TRUE;
    }
    return FALSE;
}

static bool tcam_gst_is_fourcc_yuv(const uint32_t fourcc)
{
    if (fourcc == FOURCC_YUY2
        || fourcc == FOURCC_UYVY
        //|| fourcc == FOURCC_I420
        //|| fourcc == FOURCC_YV16
        || fourcc == FOURCC_IYU1 || fourcc == FOURCC_IYU2 || fourcc == FOURCC_Y411
        || fourcc == FOURCC_NV12)
    {
        return true;
    }
    return false;
}


bool tcam::gst::format_is_yuv(const char* name, const char* fmt)
{
    if (!name || !fmt)
    {
        return false;
    }

    uint32_t fourcc = tcam::gst::tcam_fourcc_from_gst_1_0_caps_string(name, fmt);

    return tcam_gst_is_fourcc_yuv(fourcc);
}


bool tcam::gst::tcam_gst_is_bayer8_string(const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    if (strcmp(format_string, "gbrg") == 0 || strcmp(format_string, "grbg") == 0
        || strcmp(format_string, "rggb") == 0 || strcmp(format_string, "bggr") == 0)
    {
        return true;
    }

    return false;
}


bool tcam::gst::tcam_gst_is_bayer10_string(const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    if (strncmp(format_string, "gbrg10", strlen("gbrg10")) == 0
        || strncmp(format_string, "grbg10", strlen("grbg10")) == 0
        || strncmp(format_string, "rggb10", strlen("rggb10")) == 0
        || strncmp(format_string, "bggr10", strlen("bggr10")) == 0)
    {
        return true;
    }

    return false;
}


bool tcam::gst::tcam_gst_is_bayer10_packed_string(const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    static constexpr std::array<std::string_view, 12> format_list = {
        "rggb10p", "grbg10p", "gbrg10p", "bggr10p", "rggb10s", "grbg10s",
        "gbrg10s", "bggr10s", "rggb10m", "grbg10m", "gbrg10m", "bggr10m",
    };

    if (std::find(format_list.begin(), format_list.end(), std::string_view(format_string))
        != format_list.end())
    {
        return true;
    }

    return false;
}


bool tcam::gst::tcam_gst_is_bayer12_string(const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    if (strncmp(format_string, "gbrg12", strlen("gbrg12")) == 0
        || strncmp(format_string, "grbg12", strlen("grbg12")) == 0
        || strncmp(format_string, "rggb12", strlen("rggb12")) == 0
        || strncmp(format_string, "bggr12", strlen("bggr12")) == 0)
    {
        return true;
    }

    return false;
}


bool tcam::gst::tcam_gst_is_bayer12_packed_string(const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    static const std::array<std::string_view, 12> format_list = {
        "rggb12p", "grbg12p", "gbrg12p", "bggr12p", "rggb12s", "grbg12s",
        "gbrg12s", "bggr12s", "rggb12m", "grbg12m", "gbrg12m", "bggr12m",
    };

    if (std::find(format_list.begin(), format_list.end(), std::string_view(format_string))
        != format_list.end())
    {
        return true;
    }

    return false;
}

bool tcam::gst::tcam_gst_is_bayer16_string(const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    if (strcmp(format_string, "gbrg16") == 0 || strcmp(format_string, "grbg16") == 0
        || strcmp(format_string, "rggb16") == 0 || strcmp(format_string, "bggr16") == 0)
    {
        return true;
    }

    return false;
}


bool tcam::gst::tcam_gst_is_fourcc_rgb(const unsigned int fourcc)
{
    if (fourcc == GST_MAKE_FOURCC('R', 'G', 'B', 'x')
        || fourcc == GST_MAKE_FOURCC('x', 'R', 'G', 'B')
        || fourcc == GST_MAKE_FOURCC('B', 'G', 'R', 'x')
        || fourcc == GST_MAKE_FOURCC('x', 'B', 'G', 'R')
        || fourcc == GST_MAKE_FOURCC('R', 'G', 'B', 'A')
        || fourcc == GST_MAKE_FOURCC('A', 'R', 'G', 'B')
        || fourcc == GST_MAKE_FOURCC('B', 'G', 'R', 'A')
        || fourcc == GST_MAKE_FOURCC('A', 'B', 'G', 'R') || fourcc == FOURCC_BGR24
        || fourcc == FOURCC_BGRA32 || fourcc == FOURCC_BGRA64)
    {
        return TRUE;
    }

    return FALSE;
}

static bool tcam_gst_is_bayerpwl_fourcc(const uint32_t fourcc)
{
    return img::is_pwl_fcc(static_cast<img::fourcc>(fourcc));
}

static bool tcam_gst_is_polarized_mono(const unsigned int fourcc)
{
    if (fourcc == FOURCC_POLARIZATION_MONO8_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_MONO16_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_MONO12_SPACKED_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_MONO12_PACKED_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_ADI_PLANAR_MONO8
        || fourcc == FOURCC_POLARIZATION_ADI_PLANAR_MONO16
        || fourcc == FOURCC_POLARIZATION_ADI_MONO8 || fourcc == FOURCC_POLARIZATION_ADI_MONO16
        || fourcc == FOURCC_POLARIZATION_PACKED8 || fourcc == FOURCC_POLARIZATION_PACKED16)
    {
        return true;
    }

    return false;
}


static bool tcam_gst_is_polarized_bayer(const unsigned int fourcc)
{
    if (fourcc == FOURCC_POLARIZATION_BG8_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_BG16_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_BG12_SPACKED_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_BG12_PACKED_90_45_135_0
        || fourcc == FOURCC_POLARIZATION_PACKED8_BAYER_BG
        || fourcc == FOURCC_POLARIZATION_PACKED16_BAYER_BG)
    {
        return true;
    }

    return false;
}


static bool tcam_gst_fixate_caps(GstCaps* caps)
{
    if (caps == nullptr || gst_caps_is_empty(caps) || gst_caps_is_any(caps))
    {
        return FALSE;
    }

    GstStructure* structure = gst_caps_get_structure(caps, 0);

    if (gst_structure_has_field(structure, "width"))
    {
        gst_structure_fixate_field_nearest_int(structure, "width", G_MAXINT);
    }
    if (gst_structure_has_field(structure, "height"))
    {
        gst_structure_fixate_field_nearest_int(structure, "height", G_MAXINT);
    }
    if (gst_structure_has_field(structure, "framerate"))
    {
        gst_structure_fixate_field_nearest_fraction(structure, "framerate", G_MAXINT, 1);
    }

    return TRUE;
}

bool tcam::gst::gst_caps_are_bayer_only(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(caps, i);

        if (strcmp(gst_structure_get_name(struc), "video/x-bayer") != 0)
        {
            return false;
        }
    }
    return true;
}

static bool is_really_empty_caps(const GstCaps* caps)
{
    /*
gst_caps_is_empty acts erratic, thus we work around the issue with gst_caps_to_string:

--------
(gdb) print (char*)gst_caps_to_string (caps)
$4 = 0x555555a8f120 "EMPTY"
(gdb) print (int)gst_caps_is_empty (caps)
(process:5873): GStreamer-CRITICAL (recursed) **: gst_caps_is_empty: assertion 'GST_IS_CAPS (caps)' failed
--------

*/

    if (caps == nullptr)
    {
        return true;
    }

    auto tmp_caps_string = gst_helper::to_string(*caps);
    if ((tmp_caps_string == "EMPTY") || gst_caps_is_any(caps))
    {
        return true;
    }
    return false;
}


/**
 * Helper function to get a list of all available fourccs in caps
 */
static std::vector<uint32_t> index_format_fourccs(const GstCaps* caps)
{
    if (is_really_empty_caps(caps))
    {
        return {};
    }

    std::vector<uint32_t> ret;


    // only add when fourcc is not 0
    auto add_format = [&ret](const char* name, const char* fmt)
    {
        uint32_t fourcc = tcam::gst::tcam_fourcc_from_gst_1_0_caps_string(name, fmt);
        if (fourcc != 0)
        {
            ret.push_back(fourcc);
        }
    };

    for (guint i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(caps, i);

        std::vector<std::string> vec;

        if (gst_structure_get_field_type(struc, "format") == GST_TYPE_LIST)
        {
            vec = gst_list_to_vector(gst_structure_get_value(struc, "format"));
        }
        else if (gst_structure_get_field_type(struc, "format") == G_TYPE_STRING)
        {
            vec.push_back(gst_structure_get_string(struc, "format"));
        }


        const char* name = gst_structure_get_name(struc);

        if (!vec.empty())
        {
            for (const auto& fmt : vec) { add_format(name, fmt.c_str()); }
        }
        else
        {
            // this will be the case for things like image/jpeg
            // such caps will have no format field and thus vec.empty() ==
            add_format(name, "");
        }
    }

    // remove duplicate entries
    // probably never enough entries to make switch to std::set a good alternative
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());

    return ret;
}

static uint32_t find_preferred_format(const std::vector<uint32_t>& vec)
{
    using namespace tcam::gst;

    /**
     * prefer bayer 8-bit over everything else
     * if bayer 8-bit does not exist order according to the following list:
     * color formats like BGR
     * color formats like YUV
     * formats like MJPEG
     * GRAY16
     * GRAY8
     * pwl bayer
     * bayer12/16
     * polarized bayer
     * polarized mono
     */

    if (vec.empty())
    {
        return 0;
    }

    // maps are ordered
    // assume vec contains only unique values
    // map.begin() thus has the entry with best rank
    // since our key are associated in the way we prefer
    std::map<int, uint32_t> map;

    for (const auto& fourcc : vec)
    {
        if (tcam_gst_is_fourcc_bayer(fourcc))
        {
            //best_result = fourcc;
            map[0] = fourcc;
        }
        else if (tcam_gst_is_fourcc_rgb(fourcc))
        {
            map[10] = fourcc;
        }
        else if (tcam_gst_is_fourcc_yuv(fourcc))
        {
            map[20] = fourcc;
        }
        else if (fourcc == FOURCC_MJPG)
        {
            map[30] = fourcc;
        }
        else if (fourcc == FOURCC_Y800)
        {
            map[40] = fourcc;
        }
        else if (tcam_gst_is_mono10_fourcc(fourcc))
        {
            map[43] = fourcc;
        }
        else if (tcam_gst_is_mono12_fourcc(fourcc))
        {
            map[47] = fourcc;
        }
        else if (fourcc == FOURCC_Y16)
        {
            map[50] = fourcc;
        }
        else if (tcam_gst_is_bayerpwl_fourcc(fourcc))
        {
            map[60] = fourcc;
        }
        else if (tcam_gst_is_bayer10_fourcc(fourcc) || tcam_gst_is_bayer10_packed_fourcc(fourcc))
        {
            map[65] = fourcc;
        }
        else if (tcam_gst_is_bayer12_fourcc(fourcc) || tcam_gst_is_bayer12_packed_fourcc(fourcc))
        {
            map[70] = fourcc;
        }
        else if (tcam_gst_is_bayer16_fourcc(fourcc))
        {
            map[80] = fourcc;
        }
        else if (tcam_gst_is_polarized_bayer(fourcc))
        {
            map[90] = fourcc;
        }
        else if (tcam_gst_is_polarized_mono(fourcc))
        {
            map[100] = fourcc;
        }
        else
        {
            SPDLOG_ERROR("Could not associate rank with fourcc 0x{:x} {}",
                         fourcc,
                         img::fcc_to_string(fourcc).c_str());
        }
    }
    if (map.empty())
    {
        return 0;
    }

    return map.begin()->second;
}


GstCaps* tcam::gst::tcam_gst_find_largest_caps(const GstCaps* incoming, const GstCaps* filter)
{
    /**
     * find_largest_caps tries to find the largest caps
     * according to the following rules:
     *
     * 1. determine the preferred format
     *       prefer bayer 8-bit over everything else
     *       if bayer 8-bit does not exist order according to the following list:
     *       color formats like BGR
     *       formats like MJPEG
     *       GRAY8
     *       GRAY16
     *       pwl bayer
     *       bayer12/16
     *
     * 2. find the largest resolution
     * 3. for the format with the largest resolution take the highest framerate
     */
    std::vector<uint32_t> format_fourccs = index_format_fourccs(incoming);

    if (gst_caps_is_fixed(filter))
    {
        auto fccs = index_format_fourccs(filter);
        std::vector<uint32_t> tmp_fccs;

        for (const auto& fcc : format_fourccs)
        {
            if (std::any_of(fccs.begin(), fccs.end(), [fcc](uint32_t v) {return v==fcc;}))
            {
                tmp_fccs.push_back(fcc);
            }
        }

        if (!tmp_fccs.empty())
        {
            format_fourccs = tmp_fccs;
        }
    }

    uint32_t preferred_fourcc = find_preferred_format(format_fourccs);

    if (is_really_empty_caps(incoming))
    {
        return nullptr;
    }

    if (gst_caps_is_fixed(incoming))
    {
        return gst_caps_copy(incoming);
    }

    int largest_index = 0;
    int largest_width = -1;
    int largest_height = -1;
    std::string binning = "1x1";
    std::string skipping = "1x1";

    for (guint i = 0; i < gst_caps_get_size(incoming); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(incoming, i);

        const char* format = gst_structure_get_string(struc, "format");

        uint32_t fourcc =
            tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name(struc), format);

        // TODO: what about video/x-raw, format={GRAY8, GRAY16_LE}
        if (fourcc != preferred_fourcc)
        {
            continue;
        }

        int width = -1;
        int height = -1;
        bool new_width = false;
        bool new_height = false;

        // will fail if width is a range so we only handle
        // halfway fixated caps
        if (gst_structure_get_field_type(struc, "width") == G_TYPE_INT)
        {
            if (gst_structure_get_int(struc, "width", &width))
            {
                if (largest_width < width)
                {
                    largest_width = width;
                    new_width = true;
                }
            }
        }
        // else if (gst_structure_get_field_type(struc, "width") == GST_TYPE_INT_RANGE)
        // {
        //     const GValue* int_range = gst_structure_get_value(struc, "width");

        //     width = gst_value_get_int_range_max(int_range);
        //     if (largest_width < width)
        //     {
        //         largest_width = width;
        //         new_width = true;
        //     }
        // }
        else
        {
            SPDLOG_INFO("Field 'width' does not have a supported type. Current type: '{}'",
                        g_type_name(gst_structure_get_field_type(struc, "width")));
        }

        if (gst_structure_get_field_type(struc, "height") == G_TYPE_INT)
        {
            if (gst_structure_get_int(struc, "height", &height))
            {
                if (largest_height <= height)
                {
                    largest_height = height;
                    new_height = true;
                }
            }
        }
        // else if (gst_structure_get_field_type(struc, "height") == GST_TYPE_INT_RANGE)
        // {
        //     const GValue* int_range = gst_structure_get_value(struc, "height");

        //     height = gst_value_get_int_range_max(int_range);
        //     if (largest_height < height)
        //     {
        //         largest_height = height;
        //         new_height = true;
        //     }
        // }
        else
        {
            SPDLOG_INFO("Field 'height' does not have a supported type. Current type: '{}'",
                        g_type_name(gst_structure_get_field_type(struc, "height")));
        }

        if (new_width || new_height)
        {
            largest_index = i;
        }
    }

    GstCaps* largest_caps = gst_caps_copy_nth(incoming, largest_index);

    SPDLOG_INFO("Fixating assumed largest caps: {}", gst_helper::to_string(*largest_caps).c_str());

    if (gst_caps_is_fixed(largest_caps))
    {
        return largest_caps;
    }

    if (!tcam_gst_fixate_caps(largest_caps))
    {
        gst_caps_unref(largest_caps);

        GST_ERROR("Cannot fixate largest caps. Returning NULL");
        return nullptr;
    }

    GstStructure* s = gst_caps_get_structure(largest_caps, 0);

    int h;
    gst_structure_get_int(s, "height", &h);
    int w;
    gst_structure_get_int(s, "width", &w);

    int num;
    int den;
    gst_structure_get_fraction(s, "framerate", &num, &den);

    GValue vh = G_VALUE_INIT;

    g_value_init(&vh, G_TYPE_INT);
    g_value_set_int(&vh, h); // #TODO this function really looks bad here

    gst_caps_set_value(largest_caps, "height", &vh);

    GstCaps* ret_caps = gst_caps_new_simple(gst_structure_get_name(s),
                                            "framerate",
                                            GST_TYPE_FRACTION,
                                            num,
                                            den,
                                            "width",
                                            G_TYPE_INT,
                                            w,
                                            "height",
                                            G_TYPE_INT,
                                            h,
                                            "binning",
                                            G_TYPE_STRING,
                                            binning.c_str(),
                                            "skipping",
                                            G_TYPE_STRING,
                                            skipping.c_str(),
                                            NULL);

    if (gst_structure_has_field(s, "format"))
    {
        gst_caps_set_value(ret_caps, "format", gst_structure_get_value(s, "format"));
    }

    gst_caps_unref(largest_caps);

    SPDLOG_INFO("Largest caps are: {}", gst_helper::to_string(*ret_caps).c_str());

    return ret_caps;
}

bool tcam::gst::contains_jpeg(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        if (strcmp("image/jpeg", gst_structure_get_name(gst_caps_get_structure(caps, i))) == 0)
        {
            return true;
        }
    }

    return false;
}

bool tcam::gst::contains_bayer(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        if (strcmp("video/x-bayer", gst_structure_get_name(gst_caps_get_structure(caps, i))) == 0)
        {
            return true;
        }
    }

    return false;
}

bool tcam::gst::contains_mono(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        if (tcam_gst_contains_mono_8_bit(caps) || tcam_gst_contains_mono_10_bit(caps)
            || tcam_gst_contains_mono_12_bit(caps) || tcam_gst_contains_mono_16_bit(caps))

        {
            return true;
        }
    }

    return false;
}

bool tcam::gst::tcam_gst_contains_bayer_10_bit(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-bayer, format={rggb10, bggr10, gbrg10, grbg10,"
                                        "rggb10p, bggr10p, gbrg10p, grbg10p,"
                                        "rggb10s, bggr10s, gbrg10s, grbg10s,"
                                        "rggb10m, bggr10m, gbrg10m, grbg10m}");
    gboolean ret = gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


bool tcam::gst::tcam_gst_contains_bayer_12_bit(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-bayer, format={rggb12, bggr12, gbrg12, grbg12,"
                                        "rggb12p, bggr12p, gbrg12p, grbg12p,"
                                        "rggb12s, bggr12s, gbrg12s, grbg12s,"
                                        "rggb12m, bggr12m, gbrg12m, grbg12m}");
    gboolean ret = gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


bool tcam::gst::tcam_gst_contains_mono_8_bit(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-raw,format=GRAY8");
    gboolean ret = gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


bool tcam::gst::tcam_gst_contains_mono_10_bit(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-raw, format={GRAY10, GRAY10, GRAY10, GRAY10,"
                                        "GRAY10p, GRAY10p, GRAY10p, GRAY10p,"
                                        "GRAY10s, GRAY10s, GRAY10s, GRAY10s,"
                                        "GRAY10m, GRAY10m, GRAY10m, GRAY10m}");
    gboolean ret = gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


bool tcam::gst::tcam_gst_contains_mono_12_bit(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-raw, format={GRAY12, GRAY12, GRAY12, GRAY12,"
                                        "GRAY12p, GRAY12p, GRAY12p, GRAY12p,"
                                        "GRAY12s, GRAY12s, GRAY12s, GRAY12s,"
                                        "GRAY12m, GRAY12m, GRAY12m, GRAY12m}");
    gboolean ret = gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


bool tcam::gst::tcam_gst_contains_mono_16_bit(const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-raw,format=GRAY16_LE");
    gboolean ret = gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


static GstCaps* get_caps_from_element(GstElement* element, const char* padname)
{
    if (!element || !padname)
    {
        return nullptr;
    }

    auto pad = gst_element_get_static_pad(element, padname);
    GstCaps* ret = gst_pad_query_caps(pad, NULL);
    gst_object_unref(pad);

    return ret;
}

GstCaps* tcam::gst::get_caps_from_element_name(const char* elementname, const char* padname)
{
    GstElement* ele = gst_element_factory_make(elementname, "tmp-element");
    if (!ele)
    {
        return nullptr;
    }

    auto ret = get_caps_from_element(ele, padname);

    gst_object_unref(ele);

    return ret;
}


std::vector<std::string> tcam::gst::index_caps_formats(GstCaps* caps)
{
    // todo missing jpeg

    std::vector<std::string> ret;

    for (guint i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(caps, i);

        if (gst_structure_get_field_type(struc, "format") == GST_TYPE_LIST)
        {
            auto vec = gst_list_to_vector(gst_structure_get_value(struc, "format"));

            for (const auto& v : vec)
            {
                std::string str = gst_structure_get_name(struc);
                str += ",format=";
                str += v;
                ret.push_back(str);
            }
        }
        else if (gst_structure_get_field_type(struc, "format") == G_TYPE_STRING)
        {
            std::string str = gst_structure_get_name(struc);
            str += ",format=";
            str += gst_structure_get_string(struc, "format");
            ret.push_back(str);
        }
    }

    // make all entries unique
    std::sort(ret.begin(), ret.end());
    ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    return ret;
}


tcam::image_scaling tcam::gst::caps_get_scaling(GstCaps* caps)
{
    tcam::image_scaling sc = {};

    GstStructure* struc = gst_caps_get_structure(caps, 0);

    auto fill_value =
        [struc](const std::string& name, int32_t& to_fill_horizontal, int32_t& to_fill_vertical)
    {
        if (gst_structure_has_field(struc, name.c_str()))
        {
            std::string field_value = gst_structure_get_string(struc, name.c_str());

            const std::string delimiter = "x";
            std::string token_horizontal = field_value.substr(0, field_value.find(delimiter));
            std::string token_vertical = field_value.substr(field_value.find(delimiter) + 1);

            try
            {
                to_fill_horizontal = std::atoi(token_horizontal.c_str());
                to_fill_vertical = std::atoi(token_vertical.c_str());
            }
            catch (const std::exception& e)
            {
                SPDLOG_ERROR("Caught exception while interpreting {}: {}", name, e.what());

                to_fill_horizontal = 1;
                to_fill_vertical = 1;
            }
        }
        else
        {
            // SPDLOG_ERROR("No field {}. Using defaults", name);
            to_fill_horizontal = 1;
            to_fill_vertical = 1;
        }
    };

    fill_value("binning", sc.binning_h, sc.binning_v);

    fill_value("skipping", sc.skipping_h, sc.skipping_v);

    //SPDLOG_ERROR("Binning {}x{} Skipping: {}x{}", sc.binning_h, sc.binning_v, sc.skipping_h, sc.skipping_v);

    return sc;
}

bool tcam::gst::is_gst_state_equal_or_greater(GstElement* self, GstState state) noexcept
{
    GstState cur_state = GST_STATE_NULL;
    auto res = gst_element_get_state(self, &cur_state, NULL, GST_CLOCK_TIME_NONE);
    if (res == GST_STATE_CHANGE_FAILURE)
    {
        return false;
    }
    return cur_state >= state;
}

bool tcam::gst::is_gst_state_equal_or_less(GstElement* self, GstState state) noexcept
{
    GstState cur_state = GST_STATE_NULL;
    auto res = gst_element_get_state(self, &cur_state, NULL, GST_CLOCK_TIME_NONE);
    if (res == GST_STATE_CHANGE_FAILURE)
    {
        return false;
    }
    return cur_state <= state;
}
