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

#include <stddef.h> // NULL
#include <string.h> // strcmp
#include <algorithm> //std::find

GstElement* tcam_gst_find_camera_src (GstElement* element)
{

    GstElement* e = GST_ELEMENT( gst_object_get_parent(GST_OBJECT(element)));

    GList* l = GST_BIN(e)->children;
    GstElement* ret = nullptr;
    while (1==1)
    {
        const char* name = g_type_name(gst_element_factory_get_element_type(gst_element_get_factory((GstElement*)l->data)));

        if (g_strcmp0(name, "GstTcamSrc") == 0)
        {
            ret = (GstElement*)l->data;
            break;
        }

        if (g_list_next(l) == NULL)
            break;

        l = g_list_next(l);
    }

    if (ret == nullptr)
    {
        GST_ERROR("Camera source not set!");
    }
    return ret;
}


bool tcam_gst_raw_only_has_mono (const GstCaps* caps)
{
    auto correct_format = [] (const char* str)
    {
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
                if (!correct_format(gst_structure_get_string(struc, "format")))
                {
                    return false;
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


bool tcam_gst_is_fourcc_bayer (const unsigned int fourcc)
{
    if (fourcc == GST_MAKE_FOURCC('g', 'b', 'r', 'g')
        || fourcc == GST_MAKE_FOURCC('g', 'r', 'b', 'g')
        || fourcc == GST_MAKE_FOURCC('r', 'g', 'g', 'b')
        || fourcc == GST_MAKE_FOURCC('b', 'g', 'g', 'r'))
    {
        return TRUE;
    }
    return FALSE;
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
        || fourcc == GST_MAKE_FOURCC('A', 'B', 'G', 'R'))
    {
        return TRUE;
    }

    return FALSE;
}


bool tcam_gst_fixate_caps (GstCaps* caps)
{
    if (caps == nullptr || gst_caps_is_empty(caps))
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


GstCaps* tcam_gst_find_largest_caps (const GstCaps* incoming)
{
    int largest_index = -1;
    int largest_width = -1;
    int largest_height = -1;

    for (int i = 0; i < gst_caps_get_size(incoming); ++i)
    {
        GstStructure* struc = gst_caps_get_structure(incoming, i);

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

        if (gst_structure_get_int(struc, "height", &height))
        {
            if (largest_height <= height)
            {
                largest_height = height;
                new_height = true;
            }
        }

        if (new_width && new_height)
        {
            largest_index = i;
        }
    }

    GstCaps* largest_caps = gst_caps_copy_nth(incoming, largest_index);

    if (!tcam_gst_fixate_caps(largest_caps))
    {
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

    GValue vh = {};

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


bool contains_bayer (const GstCaps* caps)
{
    for (unsigned int i = 0; i < gst_caps_get_size(caps); ++i)
    {
        if (strcmp("video/x-bayer", gst_structure_get_name(gst_caps_get_structure(caps, i))) == 0)
        {
            return true;
        }
    }

    return false;
}


GstCaps* get_caps_from_element (const char* elementname, const char* padname)
{

    GstElement* ele = gst_element_factory_make(elementname, "tmp-element");

    if (!ele)
    {
        return nullptr;
    }

    GstCaps* ret = gst_pad_query_caps(gst_element_get_static_pad(ele, padname), NULL);

    gst_object_unref(ele);

    return ret;
}


GstCaps* find_input_caps (const GstCaps* available_caps,
                          const GstCaps* wanted_caps,
                          bool& requires_conversion)
{
    GstCaps* ret = nullptr;
    requires_conversion = false;

    if (!GST_IS_CAPS(available_caps))
    {
        return nullptr;
    }

    if (wanted_caps == nullptr || gst_caps_is_empty(wanted_caps))
    {
        GST_INFO("No sink caps specified. Continuing with caps from source device.");
        wanted_caps = gst_caps_copy(available_caps);
    }

    GstCaps* intersect = gst_caps_intersect(available_caps, wanted_caps);

    if (gst_caps_is_empty(intersect))
    {
        GstElementFactory* debayer = gst_element_factory_find("bayer2rgb");

        if (gst_element_factory_can_src_any_caps(debayer, wanted_caps)
            && gst_element_factory_can_sink_any_caps(debayer, available_caps))
        {
            requires_conversion = true;
            // wanted_caps can be fixed, etc.
            // thus change name to be compatible to bayer2rgb sink pad
            // and create a correct intersection
            GstCaps* temp = gst_caps_copy(wanted_caps);
            gst_caps_change_name(temp, "video/x-bayer");

            ret = gst_caps_intersect(available_caps, temp);
            gst_caps_unref(temp);
        }

        gst_object_unref(debayer);
    }
    else
    {
        return intersect;
    }

    return ret;
}


bool fill_structure_fixed_resolution (GstStructure* structure,
                                      const tcam::VideoFormatDescription& format,
                                      const tcam_resolution_description& res)
{

    std::vector<double> framerates = format.get_frame_rates(res);
    int framerate_count = framerates.size();

    GValue fps_list = {0};
    g_value_init(&fps_list, GST_TYPE_LIST);

    for (int f = 0; f < framerate_count; f++)
    {
        int frame_rate_numerator;
        int frame_rate_denominator;
        gst_util_double_to_fraction(framerates[f],
                                    &frame_rate_numerator,
                                    &frame_rate_denominator);

        GValue fraction = {0};
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


GstCaps* convert_videoformatsdescription_to_caps (std::vector<tcam::VideoFormatDescription> descriptions)
{
    GstCaps* caps = gst_caps_new_empty();

    for (const auto& desc : descriptions)
    {
        if (desc.get_fourcc() == 0)
        {
            // tcam_log(TCAM_LOG_WARNING, "Format has empty fourcc. Ignoring");
            continue;
        }

        const char* caps_string = tcam_fourcc_to_gst_1_0_caps_string(desc.get_fourcc());

        if (caps_string == nullptr)
        {
            // tcam_log(TCAM_LOG_WARNING, "Format has empty caps string. Ignoring");
            continue;
        }

        // GST_DEBUG("Found '%s' pixel format string", caps_string);

        std::vector<struct tcam_resolution_description> res = desc.get_resolutions();

        for (const auto& r : res)
        {
            int min_width = r.min_size.width;
            int min_height = r.min_size.height;

            int max_width = r.max_size.width;
            int max_height = r.max_size.height;

            if (r.type == TCAM_RESOLUTION_TYPE_RANGE)
            {              // std::vector<double> framerates = format[i].get_frame_rates(res[j]);
                std::vector<struct tcam_image_size> framesizes = tcam::get_standard_resolutions(r.min_size, r.max_size);
                framesizes.insert(framesizes.begin(), r.min_size);
                framesizes.push_back(r.max_size);
                for (const auto& reso : framesizes)
                {
                    GstStructure* structure = gst_structure_from_string (caps_string, NULL);

                    std::vector<double> framerates = desc.get_framerates(reso);

                    if (framerates.empty())
                    {
                        // tcam_log(TCAM_LOG_WARNING, "No available framerates. Ignoring format.");
                        continue;
                    }

                    GValue fps_list = {0};
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

                        GValue fraction = {0};
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

                GValue w = {};
                g_value_init(&w, GST_TYPE_INT_RANGE);
                gst_value_set_int_range(&w, min_width, max_width);

                GValue h = {};
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

                GValue f = {};
                g_value_init(&f, GST_TYPE_FRACTION_RANGE);

                gst_value_set_fraction_range_full(&f,
                                                  fps_min_num, fps_min_den,
                                                  fps_max_num, fps_max_den);

                gst_structure_set_value(structure, "width", &w);
                gst_structure_set_value(structure,"height", &h);
                gst_structure_set_value(structure,"framerate", &f);
                gst_caps_append_structure(caps, structure);}
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
