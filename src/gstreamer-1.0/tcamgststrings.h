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
#include <cstring>
#include "base_types.h"

typedef struct
{
    uint32_t fourcc;
    const char* gst_1_0_caps_string;
    const char* gst_1_0_name;
    const char* gst_format;
} TcamGstMapping;

static TcamGstMapping tcam_gst_caps_info[] =
{
    {
        FOURCC_RGB32,
        "video/x-raw, format=(string)BRGx",
        "video/x-raw",
        "BGRx",
    },
    {
        FOURCC_BGR24,
        "video/x-raw, format=(string)BGR",
        "video/x-raw",
        "BGR",
    },
    {
        FOURCC_RGB64,
        "video/x-raw, format=(string)RGBx64",
        "video/x-raw",
        "RGBx64",
    },
    {
        FOURCC_Y800,
        "video/x-raw, format=(string)GRAY8",
        "video/x-raw",
        "GRAY8",
    },
    {
        FOURCC_Y12_PACKED,
        "video/x-raw, format=(string)GRAY12p",
        "video/x-raw",
        "GRAY12p"
    },
    {
        FOURCC_Y12_SPACKED,
        "video/x-raw, format=(string)GRAY12sp",
        "video/x-raw",
        "GRAY12sp"
    },
    {
        FOURCC_Y16,
        "video/x-raw, format=(string)GRAY16_LE",
        "video/x-raw", "GRAY16_LE",
    },
    {
        FOURCC_GRBG8,
        "video/x-bayer, format=(string)grbg",
        "video/x-bayer",    "grbg",
    },
    {
        FOURCC_RGGB8,
        "video/x-bayer, format=(string)rggb",
        "video/x-bayer",    "rggb",
    },
    {
        FOURCC_GBRG8,
        "video/x-bayer, format=(string)gbrg",
        "video/x-bayer",    "gbrg",
    },
    {
        FOURCC_BGGR8,
        "video/x-bayer, format=(string)bggr",
        "video/x-bayer",    "bggr",
    },

    {
        FOURCC_GBRG10,
        "video/x-bayer, format=(string)gbrg10",
        "video/x-bayer",    "gbrg10",
    },
    {
        FOURCC_BGGR10,
        "video/x-bayer, format=(string)bggr10",
        "video/x-bayer",    "bggr10",
    },
    {
        FOURCC_GRBG10,
        "video/x-bayer, format=(string)grbg10",
        "video/x-bayer",    "grbg10",
    },
    {
        FOURCC_RGGB10,
        "video/x-bayer, format=(string)rggb10",
        "video/x-bayer",    "rggb10",
    },

    {
        FOURCC_GBRG12,
        "video/x-bayer, format=(string)gbrg12",
        "video/x-bayer",    "gbrg12",
    },
    {
        FOURCC_BGGR12,
        "video/x-bayer, format=(string)bggr12",
        "video/x-bayer",    "bggr12",
    },
    {
        FOURCC_GRBG12,
        "video/x-bayer, format=(string)grbg12",
        "video/x-bayer",    "grbg12",
    },
    {
        FOURCC_RGGB12,
        "video/x-bayer, format=(string)rggb12",
        "video/x-bayer",    "rggb12",
    },


    {
        FOURCC_GBRG12_PACKED,
        "video/x-bayer, format=(string)gbrg12p",
        "video/x-bayer",    "gbrg12p",
    },
    {
        FOURCC_BGGR12_PACKED,
        "video/x-bayer, format=(string)bggr12p",
        "video/x-bayer",    "bggr12p",
    },
    {
        FOURCC_GRBG12_PACKED,
        "video/x-bayer, format=(string)grbg12p",
        "video/x-bayer",    "grbg12p",
    },
    {
        FOURCC_RGGB12_PACKED,
        "video/x-bayer, format=(string)rggb12p",
        "video/x-bayer",    "rggb12p",
    },

    {
        FOURCC_GBRG12_SPACKED,
        "video/x-bayer, format=(string)gbrg12sp",
        "video/x-bayer",    "gbrg12sp",
    },
    {
        FOURCC_BGGR12_SPACKED,
        "video/x-bayer, format=(string)bggr12sp",
        "video/x-bayer",    "bggr12sp",
    },
    {
        FOURCC_GRBG12_SPACKED,
        "video/x-bayer, format=(string)grbg12sp",
        "video/x-bayer",    "grbg12sp",
    },
    {
        FOURCC_RGGB12_SPACKED,
        "video/x-bayer, format=(string)rggb12sp",
        "video/x-bayer",    "rggb12sp",
    },

    {
        FOURCC_GBRG12_MIPI_PACKED,
        "video/x-bayer, format=(string)gbrg12m",
        "video/x-bayer",    "gbrg12m",
    },
    {
        FOURCC_BGGR12_MIPI_PACKED,
        "video/x-bayer, format=(string)bggr12m",
        "video/x-bayer",    "bggr12m",
    },
    {
        FOURCC_GRBG12_MIPI_PACKED,
        "video/x-bayer, format=(string)grbg12m",
        "video/x-bayer",    "grbg12m",
    },
    {
        FOURCC_RGGB12_MIPI_PACKED,
        "video/x-bayer, format=(string)rggb12m",
        "video/x-bayer",    "rggb12m",
    },

    {
        FOURCC_GBRG16,
        "video/x-bayer, format=(string)gbrg16",
        "video/x-bayer",    "gbrg16",
    },
    {
        FOURCC_BGGR16,
        "video/x-bayer, format=(string)bggr16",
        "video/x-bayer",    "bggr16",
    },
    {
        FOURCC_GRBG16,
        "video/x-bayer, format=(string)grbg16",
        "video/x-bayer",    "grbg16",
    },
    {
        FOURCC_RGGB16,
        "video/x-bayer, format=(string)rggb16",
        "video/x-bayer",    "rggb16",
    },

    {
        FOURCC_YUYV,
        "video/x-raw, format=(string)YUY2",
        "video/x-raw", "YUY2",
    },
    {
        FOURCC_YUY2,
        "video/x-raw, format=(string)YUY2",
        "video/x-raw", "YUY2",
    },
    {
        FOURCC_UYVY,
        "video/x-raw, format=(string)UYVY",
        "video/x-raw",  "UYVY",
    },
    {
        FOURCC_Y422,
        "video/x-raw, format=(string)Y422",
        "video/x-raw", "Y422",
    },
    {
        FOURCC_Y411,
        "video/x-raw, format=(string)IYU1",
        "video/x-raw", "IYU1",
    },
    {
        FOURCC_MJPG,
        "image/jpeg",
        "image/jpeg", "",
    },
    {
        FOURCC_POLARIZATION_BAYER_BG8_90_45_135_0,
        "video/x-bayer, format=(string)bggr_polarized_90_45_135_0",
        "video/x-bayer", "bggr_polarized_90_45_135_0",
    },
    {
        FOURCC_POLARIZATION_BAYER_BG12_PACKED_90_45_135_0,
        "video/x-bayer, format=(string)bggr12p_polarized_90_45_135_0",
        "video/x-bayer", "bggr12p_polarized_90_45_135_0",
    },
    {
        FOURCC_POLARIZATION_BAYER_BG16_90_45_135_0,
        "video/x-bayer, format=(string)bggr16_polarized_90_45_135_0",
        "video/x-bayer", "bggr16_polarized_90_45_135_0",
    },
    {
        FOURCC_POLARIZATION_MONO8_90_45_135_0,
        "video/x-raw, format=(string)GRAY8_polarized_90_45_135_0",
        "video/x-raw", "GRAY8_polarized_90_45_135_0",
    },
    {
        FOURCC_POLARIZATION_MONO12_PACKED_90_45_135_0,
        "video/x-raw, format=(string)GRAY12p_polarized_90_45_135_0",
        "video/x-raw", "GRAY12p_polarized_90_45_135_0",
    },
    {
        FOURCC_POLARIZATION_MONO16_90_45_135_0,
        "video/x-raw, format=(string)GRAY16_LE_polarized_90_45_135_0",
        "video/x-raw", "GRAY16_LE_polarized_90_45_135_0",
    },

    // internal not meant for gst usage

    // {
    //     FOURCC_POLARIZATION_ADI_PLANAR_MONO8,
    //     "",
    //     "", ""
    // },
    // {
    //     FOURCC_POLARIZATION_ADI_PLANAR_MONO16,
    //     "",
    //     "", ""
    // },

    {
        FOURCC_POLARIZATION_ADI_MONO8,
        "video/tis,format=(string)ADI_GRAY8",
        "video/tis", "ADI_GRAY8"
    },
    {
        FOURCC_POLARIZATION_ADI_MONO16,
        "video/tis,format=(string)ADI_GRAY16_LE",
        "video/tis", "ADI_GRAY16_LE"
    },
    {
        FOURCC_POLARIZATION_ADI_RGB8,
        "video/tis,format=(string)ADI_RGB8",
        "video/tis", "ADI_RGB8"
    },
    {
        FOURCC_POLARIZATION_ADI_RGB16,
        "video/tis,format=(string)ADI_RGB16",
        "video/tis", "ADI_RGB16"
    },
    {
        FOURCC_POLARIZATION_PACKED8,
        "video/x-raw,format=(string)polarized-GREY8p",
        "video/x-raw", "polarized-GREY8p"
    },
    {
        FOURCC_POLARIZATION_PACKED16,
        "video/x-raw,format=(string)polarized-GREY16_LEp",
        "video/x-raw", "polarized-GREY16_LEp"
    },
    {
        FOURCC_POLARIZATION_PACKED8_BAYER_BG,
        "video/x-bayer,format=(string)polarized-bggr8p",
        "video/x-bayer", "polarized-bggr8p"
    },
    {
        FOURCC_POLARIZATION_PACKED16_BAYER_BG,
        "video/x-bayer,format=(string)polarized-bggr16p",
        "video/x-bayer", "polarized-bggr16p"
    },
};

#ifndef ARRAYSIZE
#define ARRAYSIZE(arr) sizeof(arr) / sizeof(arr[0])
#endif


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


const char* tcam_fourcc_to_gst_1_0_caps_name (uint32_t fourcc)
{
    for (unsigned int i = 0; i < ARRAYSIZE(tcam_gst_caps_info); ++i)
    {
        if (fourcc == tcam_gst_caps_info[i].fourcc)
        {
            return tcam_gst_caps_info[i].gst_1_0_name;
        }
    }
    return NULL;
}


const char* tcam_fourcc_to_gst_1_0_caps_format (uint32_t fourcc)
{
    for (unsigned int i = 0; i < ARRAYSIZE(tcam_gst_caps_info); ++i)
    {
        if (fourcc == tcam_gst_caps_info[i].fourcc)
        {
            return tcam_gst_caps_info[i].gst_format;
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
