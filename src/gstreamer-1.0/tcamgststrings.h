/*
* Copyright 2017 The Imaging Source Europe GmbH
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

#ifndef TCAM_GST_STRINGS_H
#define TCAM_GST_STRINGS_H

#include <stdint.h>
#include "base_types.h"

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
    },
    {
        FOURCC_YUY2,
        "video/x-raw, format=(string)YUY2",
        "video/x-raw", "YUY2",
        "video/x-raw-yuv, format=(string)yuy2, bpp=(int)8, depth=(int)8",
		"video/x-raw-yuv"
    },
    {
        FOURCC_UYVY,
		"video/x-raw, format=(string)UYVY",
		"video/x-raw",	"UYVY",
    },
    {
        FOURCC_Y422,
        "video/x-raw, format=(string)Y422",
        "video/x-raw", "Y422",
        "",
		""
    },
    {
        FOURCC_Y411,
        "video/x-raw, format=(string)IYU1",
        "video/x-raw", "IYU1",
        "", ""
    },
    {
        FOURCC_MJPG,
        "image/jpeg",
        "image/jpeg", "",
        "", ""
    },
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

#endif /* TCAM_GST_STRINGS_H */
