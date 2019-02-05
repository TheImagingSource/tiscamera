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
#include "base_types.h"
#include "logging.h"
#include "public_utils.h"
#include <stddef.h> // NULL
#include <string.h> // strcmp
#include <algorithm> //std::find

#include "tcamgststrings.h"

#include "public_utils.h"


GstElement* tcam_gst_find_camera_src (GstElement* element)
{
    GstPad* orig_pad = gst_element_get_static_pad(element, "sink");

    GstPad* src_pad = gst_pad_get_peer(orig_pad);
    g_object_unref(orig_pad);

    if (!src_pad)
    {
        // this means we have reached a dead end where no valid tcamsrc exists
        return nullptr;
    }

    GstElement* el = gst_pad_get_parent_element(src_pad);

    gst_object_unref(src_pad);
    GstElement* ret;
    const char* name = g_type_name(gst_element_factory_get_element_type(gst_element_get_factory(el)));
    if (g_strcmp0(name, "GstTcamSrc") == 0)
    {
        return el;
    }
    ret =  tcam_gst_find_camera_src(el);

    gst_object_unref(el);

    return ret;
}


std::vector<std::string> gst_list_to_vector (const GValue* gst_list)
{
    std::vector<std::string> ret;
    if (!GST_VALUE_HOLDS_LIST(gst_list))
    {
        tcam_error("Given GValue is not a list.");
        return ret;
    }

    for (unsigned int x = 0; x < gst_value_list_get_size(gst_list); ++x)
    {
        const GValue* val = gst_value_list_get_value(gst_list, x);

        if (G_VALUE_TYPE(val) == G_TYPE_STRING)
        {

            ret.push_back(g_value_get_string(val));
        }
        else
        {
            tcam_error("NOT IMPLEMENTED. TYPE CAN NOT BE INTERPRETED");
        }
    }

    return ret;
}


bool tcam_gst_raw_only_has_mono (const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    auto correct_format = [] (const char* str)
    {
        if (str == nullptr)
        {
            return false;
        }

        const static std::vector<std::string> formats = {"GRAY8", "GRAY16_LE", "GRAY16_BE"};

        if (std::find(formats.begin(), formats.end(), str) == formats.end())
        {
            return false;
        }
        return true;
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
                    tcam_error("Cannot handle format type in GstStructure.");
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


bool tcam_gst_is_fourcc_bayer (const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG8
        || fourcc == FOURCC_GRBG8
        || fourcc == FOURCC_RGGB8
        || fourcc == FOURCC_BGGR8)
    {
        return TRUE;
    }
    return FALSE;
}


bool tcam_gst_is_bayer12_fourcc (const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG12
        || fourcc == FOURCC_GRBG12
        || fourcc == FOURCC_RGGB12
        || fourcc == FOURCC_BGGR12)
    {
        return TRUE;
    }
    return FALSE;
}


bool tcam_gst_is_bayer12_packed_fourcc (const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG12_MIPI_PACKED
        || fourcc == FOURCC_GRBG12_MIPI_PACKED
        || fourcc == FOURCC_RGGB12_MIPI_PACKED
        || fourcc == FOURCC_BGGR12_MIPI_PACKED
        || fourcc == FOURCC_GBRG12_SPACKED
        || fourcc == FOURCC_GRBG12_SPACKED
        || fourcc == FOURCC_RGGB12_SPACKED
        || fourcc == FOURCC_BGGR12_SPACKED
        || fourcc == FOURCC_GBRG12_PACKED
        || fourcc == FOURCC_GRBG12_PACKED
        || fourcc == FOURCC_RGGB12_PACKED
        || fourcc == FOURCC_BGGR12_PACKED)
    {
        return TRUE;
    }
    return FALSE;
}


bool tcam_gst_is_bayer16_fourcc (const uint32_t fourcc)
{
    if (fourcc == FOURCC_GBRG16
        || fourcc == FOURCC_GRBG16
        || fourcc == FOURCC_RGGB16
        || fourcc == FOURCC_BGGR16)
    {
        return TRUE;
    }
    return FALSE;
}


bool tcam_gst_is_fourcc_mono (const uint32_t fourcc)
{
    if (fourcc == FOURCC_Y800
        || fourcc == FOURCC_Y16)
    {
        return true;
    }
    return false;
}


bool tcam_gst_is_fourcc_yuv (const uint32_t fourcc)
{
    if (fourcc == FOURCC_YUYV
        || fourcc == FOURCC_YUY2
        || fourcc == FOURCC_Y444
        || fourcc == FOURCC_Y422
        || fourcc == FOURCC_Y411)
    {
        return true;
    }
    return false;
}


bool tcam_gst_is_bayer8_string (const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    if (strcmp(format_string, "gbrg")  == 0
        || strcmp(format_string, "grbg")  == 0
        || strcmp(format_string, "rggb")  == 0
        || strcmp(format_string, "bggr")  == 0)
    {
        return true;
    }

    return false;
}


bool tcam_gst_is_bayer12_string (const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    if (strncmp(format_string, "gbrg12", strlen("gbrg12"))  == 0
        || strncmp(format_string, "grbg12", strlen("grbg12"))  == 0
        || strncmp(format_string, "rggb12", strlen("rggb12"))  == 0
        || strncmp(format_string, "bggr12", strlen("bggr12"))  == 0)
    {
        return true;
    }

    return false;
}


bool tcam_gst_is_bayer12_packed_string (const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    std::vector<std::string> format_list = {
        "rggb12p",
        "grbg12p",
        "gbrg12p",
        "bggr12p",
        "rggb12s",
        "grbg12s",
        "gbrg12s",
        "bggr12s",
        "rggb12m",
        "grbg12m",
        "gbrg12m",
        "bggr12m",
    };

    if (std::find(format_list.begin(), format_list.end(), format_string) != format_list.end())
    {
        return true;
    }

    return false;
}


bool tcam_gst_is_bayer16_string (const char* format_string)
{
    if (format_string == nullptr)
    {
        return false;
    }

    if (strcmp(format_string, "gbrg16")  == 0
        || strcmp(format_string, "grbg16")  == 0
        || strcmp(format_string, "rggb16")  == 0
        || strcmp(format_string, "bggr16")  == 0)
    {
        return true;
    }

    return false;
}


bool tcam_gst_is_fourcc_rgb (const unsigned int fourcc)
{
    if (fourcc == GST_MAKE_FOURCC('R', 'G', 'B', 'x')
        || fourcc == GST_MAKE_FOURCC('x', 'R', 'G', 'B')
        || fourcc == GST_MAKE_FOURCC('B', 'G', 'R', 'x')
        || fourcc == GST_MAKE_FOURCC('x', 'B', 'G', 'R')
        || fourcc == GST_MAKE_FOURCC('R', 'G', 'B', 'A')
        || fourcc == GST_MAKE_FOURCC('A', 'R', 'G', 'B')
        || fourcc == GST_MAKE_FOURCC('B', 'G', 'R', 'A')
        || fourcc == GST_MAKE_FOURCC('A', 'B', 'G', 'R')
        || fourcc == FOURCC_BGR24
        || fourcc == FOURCC_RGB32
        || fourcc == FOURCC_RGB64)
    {
        return TRUE;
    }

    return FALSE;
}


bool tcam_gst_fixate_caps (GstCaps* caps)
{
    if (caps == nullptr
        || gst_caps_is_empty(caps)
        || gst_caps_is_any(caps))
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


void gst_caps_change_name (GstCaps* caps, const char* name)
{
    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(caps, i);

        if (struc != nullptr)
        {
            gst_structure_set_name(struc, name);
            gst_structure_remove_field(struc, "format");
        }
    }
}


GstCaps* bayer_transform_intersect (const GstCaps* bayer, const GstCaps* raw)
{
    GstCaps* caps2 = gst_caps_copy(raw);
    gst_caps_change_name(caps2, "video/x-bayer");
    GstCaps* caps1 = gst_caps_intersect((GstCaps*)bayer, caps2);
    return caps1;
}


bool gst_caps_are_bayer_only (const GstCaps* caps)
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


/**
 * Helper function to get a list of all available fourccs in caps
 */
std::vector<uint32_t> index_format_fourccs (const GstCaps* caps)
{
    std::vector<uint32_t> ret;

    /*
    gst_caps_is_empty acts erratic, thus we work arround the issue with gst_caps_to_string:

    --------
    (gdb) print (char*)gst_caps_to_string (caps)
    $4 = 0x555555a8f120 "EMPTY"
    (gdb) print (int)gst_caps_is_empty (caps)
    (process:5873): GStreamer-CRITICAL (recursed) **: gst_caps_is_empty: assertion 'GST_IS_CAPS (caps)' failed
    --------

    */
    if (!caps || (!g_strcmp0(gst_caps_to_string(caps), "EMPTY")) || gst_caps_is_any(caps))
    {
        return ret;
    }

    for (guint i = 0; i < gst_caps_get_size(caps); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(caps, i);
        std::string format_string;
        std::vector<std::string> vec;

        if (gst_structure_get_field_type(struc, "format") == GST_TYPE_LIST)
        {
            vec = gst_list_to_vector(gst_structure_get_value(struc, "format"));
        }
        else if (gst_structure_get_field_type(struc, "format") == G_TYPE_STRING)
        {
            vec.push_back(gst_structure_get_string(struc, "format"));
        }

        // code in helper func is needed weither vec is empty or not
        // prevents code duplication
        auto helper_func = [&ret] (const char* name, const char* fmt)
            {
                uint32_t fourcc = tcam_fourcc_from_gst_1_0_caps_string(name,
                                                                       fmt);
                if (fourcc != 0)
                {
                    ret.push_back(fourcc);
                }
            };

        const char* name = gst_structure_get_name(struc);

        if (!vec.empty())
        {
            for (const auto& fmt : vec)
            {
                helper_func(name, fmt.c_str());
            }
        }
        else
        {
            // this will be the case for things like image/jpeg
            // such caps will have no format field and thus vec.empty() ==
            helper_func(name, "");
        }
    }

    // remove duplicate entries
    // probably never enough entries to make switch to std::set a good alternative
    sort( ret.begin(), ret.end() );
    ret.erase( unique( ret.begin(), ret.end() ), ret.end() );

    return ret;
}


uint32_t find_preferred_format (const std::vector<uint32_t>& vec)
{
    /**
     * prefer bayer 8-bit over everything else
     * if bayer 8-bit does not exist order according to the following list:
     * color formats like BGR
     * color formats like YUV
     * formats like MJPEG
     * GRAY16
     * GRAY8
     * bayer12/16
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
            map[1] = fourcc;
        }
        else if (tcam_gst_is_fourcc_yuv(fourcc))
        {
            map[2] = fourcc;
        }
        else if (fourcc == FOURCC_MJPG)
        {
            map[3] = fourcc;
        }
        else if (fourcc == FOURCC_Y16)
        {
            map[4] = fourcc;
        }
        else if (fourcc == FOURCC_Y800)
        {
            map[5] = fourcc;
        }
        else if (tcam_gst_is_bayer12_fourcc(fourcc) || tcam_gst_is_bayer12_packed_fourcc(fourcc))
        {
            map[6] = fourcc;
        }
        else if (tcam_gst_is_bayer16_fourcc(fourcc))
        {
            map[7] = fourcc;
        }
        else
        {
            tcam_error("Could not associate rank with fourcc %d %s", fourcc,
                       tcam::fourcc_to_description(fourcc));
        }
    }

    return map.begin()->second;
}


GstCaps* tcam_gst_find_largest_caps (const GstCaps* incoming)
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
     *       GRAY16
     *       GRAY8
     *       bayer12/16
     *
     * 2. find the largest resolution
     * 3. for the format with the largest resolution take the highest framerate
     */

    std::vector<uint32_t> format_fourccs = index_format_fourccs(incoming);

    uint32_t preferred_fourcc = find_preferred_format(format_fourccs);

    if(!g_strcmp0(gst_caps_to_string(incoming), "EMPTY"))
    {
        return nullptr;
    }

    int largest_index = -1;
    int largest_width = -1;
    int largest_height = -1;

    for (guint i = 0; i < gst_caps_get_size(incoming); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(incoming, i);

        const char* format = gst_structure_get_string(struc, "format");

        uint32_t fourcc = tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name(struc), format);

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
        if (gst_structure_get_int(struc, "width", &width))
        {
            if (largest_width <= width)
            {
                largest_width = width;
                new_width = true;
            }
        }
        else
        {
            tcam_warning("Field 'width' does not have the type 'int'");
        }

        if (gst_structure_get_int(struc, "height", &height))
        {
            if (largest_height <= height)
            {
                largest_height = height;
                new_height = true;
            }
        }
        else
        {
            tcam_warning("Field 'height' does not have the type 'int'");
        }

        if (new_width && new_height)
        {
            largest_index = i;
        }
    }

    GstCaps* largest_caps = gst_caps_copy_nth(incoming, largest_index);

    tcam_info("Fixating assumed largest caps: %s", gst_caps_to_string(largest_caps));

    if (!tcam_gst_fixate_caps(largest_caps))
    {
        tcam_error("Cannot fixate largest caps. Returning NULL");
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
    g_value_set_int(&vh, h);

    gst_caps_set_value(largest_caps, "height", &vh);
    largest_caps = gst_caps_new_simple (gst_structure_get_name(s),
                                        "framerate", GST_TYPE_FRACTION, num, den,
                                        "width", G_TYPE_INT, w,
                                        "height", G_TYPE_INT, h,
                                        NULL);

    if (gst_structure_has_field(s, "format"))
    {
        gst_caps_set_value(largest_caps, "format", gst_structure_get_value(s, "format"));
    }

    return largest_caps;
}

bool contains_jpeg (const GstCaps* caps)
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

bool contains_bayer (const GstCaps* caps)
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

bool tcam_gst_can_intersect_simple(const GstCaps *caps, const gchar *capsstring)
{
    GstCaps *tmpcaps = gst_caps_from_string(capsstring);
    gboolean ret = gst_caps_can_intersect(caps, tmpcaps);
    gst_caps_unref(tmpcaps);

    return ret;
}


bool tcam_gst_contains_bayer_8_bit (const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }


    GstCaps* tmp = gst_caps_from_string("video/x-bayer, format={rggb, bggr, gbrg, grbg}");
    gboolean ret =  gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


bool tcam_gst_contains_bayer_12_bit (const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-bayer, format={rggb12, bggr12, gbrg12, grbg12,"
                                                               "rggb12p, bggr12p, gbrg12p, grbg12p,"
                                                               "rggb12s, bggr12s, gbrg12s, grbg12s,"
                                                               "rggb12m, bggr12m, gbrg12m, grbg12m}");
    gboolean ret =  gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


bool tcam_gst_contains_bayer_16_bit (const GstCaps* caps)
{
    if (caps == nullptr)
    {
        return false;
    }

    GstCaps* tmp = gst_caps_from_string("video/x-bayer, format={rggb16, bggr16, gbrg16, grbg16}");
    gboolean ret =  gst_caps_can_intersect(caps, tmp);
    gst_caps_unref(tmp);

    return ret;
}


GstCaps* get_caps_from_element (GstElement* element, const char* padname)
{
    if (!element || !padname)
    {
        return nullptr;
    }

    GstCaps* ret = gst_pad_query_caps(gst_element_get_static_pad(element, padname), NULL);

    return ret;
}


GstCaps* get_caps_from_element_name (const char* elementname, const char* padname)
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


std::vector<std::string> index_caps_formats (GstCaps* caps)
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
    if (ret.size() > 1)
    {
        std::sort(ret.begin(), ret.end());

        ret.erase(std::unique(ret.begin(), ret.end()), ret.end());
    }
    return std::move(ret);
}


/**
 * @param formats - GstCaps from which the format and name shall be used
 * @param rest - GstCaps from which the rest i.e. width,height,framerate, shall be used, is_fixed has to be true
 * @return GstCaps containing merged caps, user takes ownership
 */
GstCaps* create_caps_for_formats (GstCaps* formats, GstCaps* rest)
{
    if (!gst_caps_is_fixed(rest))
    {
        return nullptr;
    }

    auto st = gst_caps_get_structure(rest, 0);
    auto width = gst_structure_get_value(st, "width");
    auto height = gst_structure_get_value(st, "height");
    auto framerate = gst_structure_get_value(st, "framerate");

    auto caps_formats = index_caps_formats(formats);

    if (caps_formats.empty())
    {
        tcam_error("Could not identify formats for caps creation");
        return nullptr;
    }

    GstCaps* ret = gst_caps_new_empty();

    for (const auto& fmt : caps_formats)
    {
        GstCaps* tmp = gst_caps_from_string(fmt.c_str());

        if (width)
        {
            gst_caps_set_value(tmp, "width", width);
        }
        if (height)
        {
            gst_caps_set_value(tmp, "height", height);
        }
        if (framerate)
        {
            gst_caps_set_value(tmp, "framerate", framerate);
        }
        tcam_info("%s", gst_caps_to_string(tmp));

        gst_caps_append(ret, tmp);
    }

    return ret;
}


GstCaps* find_input_caps_dutils (GstCaps* available_caps,
                                 GstCaps* wanted_caps,
                                 bool& /*requires_bayer*/,
                                 bool& requires_vidoeconvert,
                                 bool& /*requires_jpegdec*/,
                                 bool& requires_dutils,
                                 bool& requires_biteater)
{
    requires_vidoeconvert = true;

    GstElementFactory* dutils = gst_element_factory_find("tcamdutils");

    if (dutils)
    {
        // check if only dutils suffice
        if (gst_element_factory_can_src_any_caps(dutils, wanted_caps)
            && gst_element_factory_can_sink_any_caps(dutils, available_caps))
        {
            requires_dutils = true;
            gst_object_unref(dutils);

            GstElementFactory* biteater = gst_element_factory_find("tcambiteater");
            // check if dutils + biteater suffice
            if (!tcam_gst_contains_bayer_8_bit(available_caps)
                && ((tcam_gst_contains_bayer_12_bit(available_caps)
                     || tcam_gst_contains_bayer_16_bit(available_caps)))
                && gst_element_factory_can_src_any_caps(biteater, wanted_caps))
            {
                requires_biteater = true;

                gst_object_unref(biteater);

                return available_caps;
            }
            else // dutils only
            {
                gst_object_unref(biteater);
                GstCaps* ret;
                if (!gst_caps_is_fixed(available_caps))
                {
                    if (!gst_caps_is_empty(wanted_caps) &&
                        g_strcmp0(gst_caps_to_string(wanted_caps), "NULL") != 0)
                    {
                        GstCaps* possible_matches = create_caps_for_formats(available_caps, wanted_caps);

                        if (!possible_matches || gst_caps_is_empty(possible_matches))
                        {
                            tcam_error("No possible matches for dutils.");
                            return nullptr;
                        }

                        ret = gst_caps_intersect(available_caps, possible_matches);
                        gst_caps_unref(possible_matches);
                    }
                    else
                    {
                        ret = tcam_gst_find_largest_caps(available_caps);
                    }
                }
                else // is fixed
                {
                    ret = gst_caps_copy(available_caps);
                }

                if (!ret)
                {
                    // TODO not compatible
                    tcam_error("No intersecting caps between dutils and src");
                    return nullptr;
                }

                return ret;
            }
            gst_object_unref(biteater);
        }
        gst_object_unref(dutils);
    }

    tcam_error("Could not create dutils.");
    return nullptr;
}


/**
 * This function works as follows (please edit when changin)
 * Check if input directly supports wanted caps
 * if yes -> get intersect and be done
 * check if conversions are required.
 * jpegdec
 * debayer
 *
 * generally speaking prefer (for both input/output)
 *                           color over mono
 *                           RGBx over YUV
 *                           anything over jpeg
 */

// TODO: bayer and videoconvert should consider eachother
GstCaps* find_input_caps (GstCaps* available_caps,
                          GstCaps* wanted_caps,
                          bool& requires_bayer,
                          bool& requires_vidoeconvert,
                          bool& requires_jpegdec,
                          bool& requires_dutils,
                          bool& requires_biteater,
                          bool use_dutils
    )
{
    requires_bayer = false;
    requires_vidoeconvert = false;
    requires_jpegdec = false;
    requires_dutils = false;
    requires_biteater = false;
    requires_bayer = false;

    if (!GST_IS_CAPS(available_caps))
    {
        return nullptr;
    }

    if (wanted_caps == nullptr || gst_caps_is_empty(wanted_caps))
    {
        GST_INFO("No sink caps specified. Continuing with caps from source device.");
        wanted_caps = gst_caps_copy(available_caps);
    }

    // GstCaps* intersect = gst_caps_intersect(available_caps, wanted_caps);
    // if (!gst_caps_is_empty(intersect))
    // {
    //     return intersect;
    // }
    // gst_caps_unref(intersect);
    GstElementFactory* dutils = gst_element_factory_find("tcamdutils");
    if (use_dutils && dutils)
    {
        gst_object_unref(dutils);
        return find_input_caps_dutils(available_caps,
                                      wanted_caps,
                                      requires_bayer,
                                      requires_vidoeconvert,
                                      requires_jpegdec,
                                      requires_dutils,
                                      requires_biteater);
    }

    GstElementFactory* debayer = gst_element_factory_find("bayer2rgb");

    if (debayer)
    {
        if (gst_element_factory_can_src_any_caps(debayer, wanted_caps)
            && gst_element_factory_can_sink_any_caps(debayer, available_caps))
        {
            requires_bayer = true;
            // wanted_caps can be fixed, etc.
            // thus change name to be compatible to bayer2rgb sink pad
            // and create a correct intersection
            GstCaps* temp = gst_caps_copy(wanted_caps);
            gst_caps_change_name(temp, "video/x-bayer");

            GstCaps* ret = gst_caps_intersect(available_caps, temp);
            gst_caps_unref(temp);
            gst_object_unref(debayer);
            return ret;
        }
    }
    gst_object_unref(debayer);

    GstElementFactory* convert = gst_element_factory_find("videoconvert");

    if (convert)
    {
        if (gst_element_factory_can_src_any_caps(convert, wanted_caps)
            && gst_element_factory_can_sink_any_caps(convert, available_caps))
        {
            // this intersection check is to ensure that we can't just pass
            // the caps through and really need this additional element
            GstCaps* intersect = gst_caps_intersect(available_caps, wanted_caps);
            if (!gst_caps_is_empty(intersect))
            {
                return intersect;
            }
            gst_caps_unref(intersect);

            requires_vidoeconvert = true;
            // wanted_caps can be fixed, etc.

            GstCaps* in = get_caps_from_element_name("videoconvert", "sink");

            // this removes things like jpeg
            GstCaps* ret = gst_caps_intersect(available_caps, in);

            GstCaps* temp = gst_caps_copy(wanted_caps);
            for (unsigned int i = 0; i < gst_caps_get_size(temp); ++i)
            {
                gst_structure_remove_field(gst_caps_get_structure(temp, i), "format");
            }

            GstCaps* ret2 = gst_caps_intersect(ret, temp);
            gst_caps_unref(in);
            gst_caps_unref(ret);
            gst_object_unref(convert);
            return ret2;
        }
        gst_object_unref(convert);

    }

    GstElementFactory* jpegdec = gst_element_factory_find("jpegdec");

    if (jpegdec)
    {
        if (gst_element_factory_can_src_any_caps(jpegdec, wanted_caps)
            && gst_element_factory_can_sink_any_caps(jpegdec, available_caps))
        {
            requires_jpegdec = true;
            requires_vidoeconvert = true;
            // wanted_caps can be fixed, etc.
            // thus change name to be compatible to jpegdec sink pad
            // and create a correct intersection
            GstCaps* temp = gst_caps_copy(wanted_caps);
            gst_caps_change_name(temp, "image/jpeg");

            for (unsigned int i = 0; i < gst_caps_get_size(temp); ++i)
            {
                gst_structure_remove_field(gst_caps_get_structure(temp, i), "format");
            }

            GstCaps* ret = gst_caps_intersect(available_caps, temp);
            gst_caps_unref(temp);
            gst_object_unref(jpegdec);
            return ret;
        }

        gst_object_unref(jpegdec);
    }
    GstCaps* intersect = gst_caps_intersect(available_caps, wanted_caps);
    if (!gst_caps_is_empty(intersect))
    {
        return intersect;
    }
    gst_caps_unref(intersect);

    return nullptr;
}


bool fill_structure_fixed_resolution (GstStructure* structure,
                                      const tcam::VideoFormatDescription& format,
                                      const tcam_resolution_description& res)
{

    std::vector<double> framerates = format.get_frame_rates(res);
    int framerate_count = framerates.size();

    GValue fps_list = G_VALUE_INIT;
    g_value_init(&fps_list, GST_TYPE_LIST);

    for (int f = 0; f < framerate_count; f++)
    {
        int frame_rate_numerator;
        int frame_rate_denominator;
        gst_util_double_to_fraction(framerates[f],
                                    &frame_rate_numerator,
                                    &frame_rate_denominator);

        GValue fraction = G_VALUE_INIT;
        g_value_init(&fraction, GST_TYPE_FRACTION);
        gst_value_set_fraction(&fraction, frame_rate_numerator, frame_rate_denominator);
        gst_value_list_append_value(&fps_list, &fraction);
        g_value_unset(&fraction);
    }

    gst_structure_set (structure,
                       "width", G_TYPE_INT, res.max_size.width,
                       "height", G_TYPE_INT, res.max_size.height,
                       NULL);

    gst_structure_take_value(structure, "framerate", &fps_list);

    return true;
}


GstCaps* convert_videoformatsdescription_to_caps (const std::vector<tcam::VideoFormatDescription>& descriptions)
{
    GstCaps* caps = gst_caps_new_empty();

    for (const auto& desc : descriptions)
    {
        if (desc.get_fourcc() == 0)
        {
            tcam_info("Format has empty fourcc. Ignoring");
            continue;
        }

        const char* caps_string = tcam_fourcc_to_gst_1_0_caps_string(desc.get_fourcc());

        if (caps_string == nullptr)
        {
            tcam_warning("Format has empty caps string. Ignoring %s",
                         tcam::fourcc_to_description(desc.get_fourcc()));
            continue;
        }

        // tcam_error("Found '%s' pixel format string", caps_string);

        std::vector<struct tcam_resolution_description> res = desc.get_resolutions();

        for (const auto& r : res)
        {
            int min_width = r.min_size.width;
            int min_height = r.min_size.height;

            int max_width = r.max_size.width;
            int max_height = r.max_size.height;

            if (r.type == TCAM_RESOLUTION_TYPE_RANGE)
            {
                std::vector<struct tcam_image_size> framesizes = tcam::get_standard_resolutions(r.min_size,
                                                                                                r.max_size);

                // check if min/max are already in the vector.
                // some devices return std resolutions as max
                if (r.min_size != framesizes.front())
                {
                    framesizes.insert(framesizes.begin(), r.min_size);
                }

                if (r.max_size != framesizes.back())
                {
                    framesizes.push_back(r.max_size);
                }

                for (const auto& reso : framesizes)
                {
                    GstStructure* structure = gst_structure_from_string (caps_string, NULL);

                    std::vector<double> framerates = desc.get_framerates(reso);

                    if (framerates.empty())
                    {
                        // tcam_log(TCAM_LOG_WARNING, "No available framerates. Ignoring format.");
                        continue;
                    }

                    GValue fps_list = G_VALUE_INIT;
                    g_value_init(&fps_list, GST_TYPE_LIST);

                    for (const auto& f : framerates)
                    {
                        int frame_rate_numerator;
                        int frame_rate_denominator;
                        gst_util_double_to_fraction(f,
                                                    &frame_rate_numerator,
                                                    &frame_rate_denominator);

                        if ((frame_rate_denominator == 0) || (frame_rate_numerator == 0))
                        {
                            continue;
                        }

                        GValue fraction = G_VALUE_INIT;
                        g_value_init(&fraction, GST_TYPE_FRACTION);
                        gst_value_set_fraction(&fraction, frame_rate_numerator, frame_rate_denominator);
                        gst_value_list_append_value(&fps_list, &fraction);
                        g_value_unset(&fraction);
                    }


                    gst_structure_set (structure,
                                       "width", G_TYPE_INT, reso.width,
                                       "height", G_TYPE_INT, reso.height,
                                       NULL);

                    gst_structure_take_value(structure, "framerate", &fps_list);
                    gst_caps_append_structure (caps, structure);

                }

                // finally also add the range to allow unusual settings like 1920x96@90fps
                GstStructure* structure = gst_structure_from_string (caps_string, NULL);

                GValue w = G_VALUE_INIT;
                g_value_init(&w, GST_TYPE_INT_RANGE);
                gst_value_set_int_range(&w, min_width, max_width);

                GValue h = G_VALUE_INIT;
                g_value_init(&h, GST_TYPE_INT_RANGE);
                gst_value_set_int_range(&h, min_height, max_height);

                std::vector<double> fps = desc.get_frame_rates(r);

                if (fps.empty())
                {
                    // GST_ERROR("Could not find any framerates for format");
                    continue;
                }

                int fps_min_num;
                int fps_min_den;
                int fps_max_num;
                int fps_max_den;
                gst_util_double_to_fraction(*std::min_element(fps.begin(), fps.end()),
                                            &fps_min_num,
                                            &fps_min_den);
                gst_util_double_to_fraction(*std::max_element(fps.begin(), fps.end()),
                                            &fps_max_num,
                                            &fps_max_den);

                GValue f = G_VALUE_INIT;
                g_value_init(&f, GST_TYPE_FRACTION_RANGE);

                gst_value_set_fraction_range_full(&f,
                                                  fps_min_num, fps_min_den,
                                                  fps_max_num, fps_max_den);

                gst_structure_set_value(structure, "width", &w);
                gst_structure_set_value(structure,"height", &h);
                gst_structure_set_value(structure,"framerate", &f);
                gst_caps_append_structure(caps, structure);
            }
            else
            {
                GstStructure* structure = gst_structure_from_string (caps_string, NULL);

                fill_structure_fixed_resolution(structure, desc, r);
                gst_caps_append_structure (caps, structure);
            }
        }

    }

    return caps;
}


bool videoformatsdescription_to_gst_caps_string (const std::vector<tcam::VideoFormatDescription>& descriptions,
                                                 std::string& str)
{
    GstCaps* caps = convert_videoformatsdescription_to_caps(descriptions);

    if (caps == nullptr || gst_caps_is_empty(caps))
    {
        return false;
    }

    const char* tmp = gst_caps_to_string(caps);

    if (tmp == nullptr)
    {
        gst_caps_unref(caps);
        return false;
    }

    str = tmp;
    gst_caps_unref(caps);

    return true;
}


bool gst_caps_to_tcam_video_format (GstCaps* caps, struct tcam_video_format* format)
{
    if (!caps || !gst_caps_is_fixed(caps) || !format)
    {
        return false;
    }

    *format = {};

    GstStructure* struc = gst_caps_get_structure(caps, 0);

    format->fourcc = tcam_fourcc_from_gst_1_0_caps_string(gst_structure_get_name(struc),
                                                          gst_structure_get_string(struc, "format"));

    gint tmp_w, tmp_h;
    gst_structure_get_int(struc, "width", &tmp_w);
    gst_structure_get_int(struc, "height", &tmp_h);
    format->width = tmp_w < 0 ? 0 : tmp_w;
    format->height = tmp_h < 0 ? 0 : tmp_h;

    int num;
    int den;
    gst_structure_get_fraction(struc, "framerate", &num, &den);

    format->framerate = den / num;

    return true;
}
