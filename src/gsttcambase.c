

#include "gsttcambase.h"

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
		"video/x-raw, format=(string)ARGB",
		"video/x-raw",
        "ARGB",
		"video/x-raw-rgb, bpp=(int)32, depth=(int)8",
		"video/x-raw-rgb"
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
            if (strcmp(format, tcam_gst_caps_info[i].gst_format) == 0)
                return tcam_gst_caps_info[i].fourcc;
        }
    }
    return 0;
}

