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


bool tcam_gst_raw_only_has_mono (const GstCaps* src_caps)
{
    GstCaps* raw_filter = gst_caps_from_string("video/x-raw");

    GstCaps* caps = gst_caps_intersect(src_caps, raw_filter);

    gst_caps_unref(raw_filter);

    GstCaps* filter = gst_caps_from_string("video/x-raw,format={GRAY8, GRAY16_LE}; ANY");

    GstCaps* sub = gst_caps_subtract(caps, filter);

    gst_caps_unref(filter);

    if (gst_caps_is_empty(sub))
    {
        gst_caps_unref(sub);
        return TRUE;
    }
    gst_caps_unref(sub);

    gst_caps_unref(caps);

    return FALSE;
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
            if (largest_width < width)
            {
                largest_width = width;
                new_width = true;
            }
        }

        if (gst_structure_get_int(struc, "height", &height))
        {
            if (largest_height < height)
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

    return largest_caps;
}

// bool gst_buffer_to_tcam_image_buffer(GstBuffer* buffer, tcam_image_buffer* buf)
// {
//     if (buffer == NULL || buf == NULL)
//     {
//         return false;
//     }

//     // *buf = {};

//     GstMapInfo info;

//     gst_buffer_map(buffer, &info, GST_MAP_READ);

//     buf->length = info.size;
//     buf->pData = info.data

//     return false;
// }
