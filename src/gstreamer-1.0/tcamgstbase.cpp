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

typedef struct
{
    uint32_t fourcc;
    const char* gst_1_0_caps_string;
    const char* gst_1_0_name;
    const char* gst_format;
    const char* gst_0_10_caps_string;
    const char* gst_0_10_name;
} TcamGstMapping;

static TcamGstMapping tcam_gst_caps_info[] =
{
	{
		FOURCC_RGB32,
		"video/x-raw, format=(string)RGB",
		"video/x-raw",
        "RGB",
		"video/x-raw-rgb, bpp=(int)32, depth=(int)8",
		"video/x-raw-rgb"
	},
	{
		FOURCC_BGR24,
		"video/x-raw, format=(string)BGR",
		"video/x-raw",
        "BGR",
		"video/x-raw-bgr, bpp=(int)24, depth=(int)8",
		"video/x-raw-bgr"
	},
	{
		FOURCC_Y800,
		"video/x-raw, format=(string)GRAY8",
		"video/x-raw",
        "GRAY8",
		"video/x-raw-gray, bpp=(int)8, depth=(int)8",
		"video/x-raw-gray"
	},
	/* { */
	/* 	FOURCC_Y16, */
	/* 	"video/x-raw, format=(string)GRAY16_LE", */
	/* 	"video/x-raw", 	"GRAY16_LE", */
	/* 	"video/x-raw-gray, bpp=(int)16, depth=(int)10", */
	/* 	"video/x-raw-gray" */
	/* }, */
	/* { */
    /*     FOURCC_Y16, */
	/* 	"video/x-raw, format=(string)GRAY16_LE", */
	/* 	"video/x-raw", 	"GRAY16_LE", */
	/* 	"video/x-raw-gray, bpp=(int)16, depth=(int)10", */
	/* 	"video/x-raw-gray" */
	/* }, */
	{
        FOURCC_Y16,
        "video/x-raw, format=(string)GRAY16_LE",
		"video/x-raw", 	"GRAY16_LE",
		"video/x-raw-gray, bpp=(int)16, depth=(int)10",
		"video/x-raw-gray"
	},
	{
		FOURCC_GRBG8,
		"video/x-bayer, format=(string)grbg",
		"video/x-bayer",	"grbg",
		"video/x-raw-bayer, format=(string)grbg, bpp=(int)8, depth=(int)8",
		"video/x-raw-bayer"
	},
	{
		FOURCC_RGGB8,
		"video/x-bayer, format=(string)rggb",
		"video/x-bayer",	"rggb",
		"video/x-raw-bayer, format=(string)rggb, bpp=(int)8, depth=(int)8",
		"video/x-raw-bayer"
	},
	{
		FOURCC_GBRG8,
		"video/x-bayer, format=(string)gbrg",
		"video/x-bayer",	"gbrg",
		"video/x-raw-bayer, format=(string)gbrg, bpp=(int)8, depth=(int)8",
		"video/x-raw-bayer"
	},
	{
		FOURCC_BGGR8,
		"video/x-bayer, format=(string)bggr",
		"video/x-bayer",	"bggr",
		"video/x-raw-bayer, format=(string)bggr, bpp=(int)8, depth=(int)8",
		"video/x-raw-bayer"
	},
    {
        FOURCC_YUYV,
        "video/x-raw, format=(string)YUY2",
        "video/x-raw", "YUY2",
        "video/x-raw-yuv, format=(string)yuy2, bpp=(int)8, depth=(int)8",
		"video/x-raw-yuv"
        // FOURCC_YUYV,
        // "video/x-raw, format=(string)YUY2",
        // "video/x-raw", "YUY2",
        // "video/x-raw-yuv, format=(string)yuy2, bpp=(int)8, depth=(int)8",
		// "video/x-raw-yuv"
    },
    {
        FOURCC_MJPG,
        "image/jpeg",
        "image/jpeg", "",
        "", ""
    },

/* Non 8bit bayer formats are not supported by gstreamer bayer plugin.
 * This feature is discussed in bug https://bugzilla.gnome.org/show_bug.cgi?id=693666 .*/

	/* { */
	/* 	ARV_PIXEL_FORMAT_YUV_422_PACKED, */
	/* 	"video/x-raw, format=(string)UYVY", */
	/* 	"video/x-raw",	"UYVY", */
	/* 	"video/x-raw-yuv, format=(fourcc)UYVY", */
	/* 	"video/x-raw-yuv",	0,	0,	ARV_MAKE_FOURCC ('U','Y','V','Y') */
	/* }, */
	/* { */
	/* 	ARV_PIXEL_FORMAT_YUV_422_YUYV_PACKED, */
	/* 	"video/x-raw, format=(string)YUY2", */
	/* 	"video/x-raw-yuv, format=(fourcc)YUYU2", */
	/* 	"video/x-raw-yuv",	0,	0,	ARV_MAKE_FOURCC ('Y','U','Y','2') */
	/* }, */
	/* { */
	/* 	ARV_PIXEL_FORMAT_RGB_8_PACKED, */
	/* 	"video/x-raw, format=(string)RGB", */
	/* 	"video/x-raw",	"RGB", */
	/* 	"video/x-raw-rgb, format=(string)RGB, bpp=(int)24, depth=(int)24", */
	/* 	"video/x-raw-rgb",	24,	24,	0 */
	/* }, */
	/* { */
	/* 	ARV_PIXEL_FORMAT_CUSTOM_YUV_422_YUYV_PACKED, */
	/* 	"video/x-raw, format=(string)YUY2", */
	/* 	"video/x-raw",	"YUY2", */
	/* 	"video/x-raw-yuv, format=(fourcc)YUYU2", */
	/* 	"video/x-raw-yuv",	0,	0,	ARV_MAKE_FOURCC ('Y','U','Y','2') */
	/* } */
};

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr) sizeof(arr) / sizeof(arr[0])
#endif


const char* tcam_fourcc_to_gst_0_10_caps_string (uint32_t fourcc)
{
	unsigned int i;
	for (i = 0; i < ARRAYSIZE(tcam_gst_caps_info); ++i)
	{
		if (fourcc == tcam_gst_caps_info[i].fourcc)
		{
			return tcam_gst_caps_info[i].gst_0_10_caps_string;
		}
	}
	return NULL;
}


uint32_t tcam_fourcc_from_gst_0_10_caps_string (const char* name, const char* format)
{

	unsigned int i;

	for (i = 0; i < ARRAYSIZE(tcam_gst_caps_info); ++i)
	{
		if (strcmp(name, tcam_gst_caps_info[i].gst_0_10_name) == 0)
		{
			if (strcmp(format, tcam_gst_caps_info[i].gst_format) == 0)
				return tcam_gst_caps_info[i].fourcc;
		}
	}
	return 0;
}

const char* tcam_fourcc_to_gst_1_0_caps_string (uint32_t fourcc)
{
    unsigned int i;
    for (i = 0; i < ARRAYSIZE(tcam_gst_caps_info); ++i)
    {
        if (fourcc == tcam_gst_caps_info[i].fourcc)
        {
            return tcam_gst_caps_info[i].gst_1_0_caps_string;
        }
    }
    return NULL;
}


uint32_t tcam_fourcc_from_gst_1_0_caps_string (const char* name, const char* format)
{

    unsigned int i;

    for (i = 0; i < ARRAYSIZE(tcam_gst_caps_info); ++i)
    {
        if (strcmp(name, tcam_gst_caps_info[i].gst_1_0_name) == 0)
        {
            if (format != NULL)
            {
                if (strcmp(format, tcam_gst_caps_info[i].gst_format) == 0)
                {
                    return tcam_gst_caps_info[i].fourcc;
                }
            }
            else
            {
                return tcam_gst_caps_info[i].fourcc;
            }
        }
    }
    return 0;
}


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


gboolean tcam_gst_raw_only_has_mono (const GstCaps* src_caps)
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


gboolean tcam_gst_is_fourcc_bayer (const guint fourcc)
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


gboolean tcam_gst_is_fourcc_rgb (const guint fourcc)
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


gboolean tcam_gst_fixate_caps (GstCaps* caps)
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
