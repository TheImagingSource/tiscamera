/*
 * Copyright 2015 The Imaging Source Europe GmbH
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

#include "format.h"

#include "image_fourcc.h"
#include "logging.h"

#include <cstring>

struct fourcc_names
{
    uint32_t fourcc;
    char name[24];
};


fourcc_names fourcc_name_array [] =
{
    {FOURCC_RGB8, "RGB8"},
    {FOURCC_RGB24, "RGB24"},
    {FOURCC_RGB32, "RGB32"},
    {FOURCC_RGB64, "RGB64"},

    {FOURCC_YUY2, "YUY2"},
    {FOURCC_Y800, "Y800"},
    {FOURCC_Y12P, "Y12 packed"},
    {FOURCC_BY8,   "BY8"},
    {FOURCC_UYVY, "UYVY"},
    {FOURCC_YUYV, "YUYV"},
    {FOURCC_YGB0, "YGB0"},
    {FOURCC_YGB1, "YGB1"},
    {FOURCC_Y16,  "Y16"},

    {FOURCC_Y444, "Y444"},
    {FOURCC_Y411, "Y411"},
    {FOURCC_IYU1, "Y411_8"},

    {FOURCC_BGGR8, "BA81"},
    {FOURCC_GBRG8, "GBRG"},
    {FOURCC_GRBG8, "GRBG"},
    {FOURCC_RGGB8, "RGGB"},

    {FOURCC_BGGR10, "BG10"},
    {FOURCC_GBRG10, "GB10"},
    {FOURCC_GRBG10, "BA10"},
    {FOURCC_RGGB10, "RG10"},

    {FOURCC_BGGR12, "BG12"},
    {FOURCC_GBRG12, "GB12"},
    {FOURCC_GRBG12, "BA12"},
    {FOURCC_RGGB12, "RG12"},

    {FOURCC_BGGR12_PACKED, "BG12 Packed"},
    {FOURCC_GBRG12_PACKED, "GB12 Packed"},
    {FOURCC_GRBG12_PACKED, "BA12 Packed"},
    {FOURCC_RGGB12_PACKED, "RG12 Packed"},

    {FOURCC_BGGR16, "BG16"},
    {FOURCC_GBRG16, "GB16"},
    {FOURCC_GRBG16, "BA16"},
    {FOURCC_RGGB16, "RG16"},

    {FOURCC_I420, "I420"},
    {FOURCC_YV16, "YV16"},
    {FOURCC_YUV8PLANAR, "YUV8 planar"},
    {FOURCC_YUV16PLANAR, "YUV16 planar"},
    {FOURCC_YUV8, "YUV8"},
    {FOURCC_H264, "H264"},
    {FOURCC_MJPG, "MJPG"},
};


const char* tcam::fourcc2description (uint32_t fourcc)
{
    for (const auto& entry : fourcc_name_array)
    {
        if (entry.fourcc == fourcc)
        {
            return entry.name;
        }
    }

    tcam_log(TCAM_LOG_ERROR, "No string for fourcc %x", fourcc);

    return "";
}


uint32_t tcam::description2fourcc (const char* description)
{
    for (const auto& entry : fourcc_name_array)
    {
        if (strcmp(entry.name, description) == 0)
        {
            return entry.fourcc;
        }
    }
    return 0;
}


std::string tcam::fourcc2string (uint32_t fourcc)
{


    union _bla
    {
        uint32_t i;
        char c[4];
    } bla;

    bla.i = fourcc;

    std::string s (bla.c);

    // std::string s ( (char*)&fourcc);
    // s += "\0";
    return s;
}


uint32_t tcam::string2fourcc (const std::string& s)
{
    if(s.length() != 4)
    {
        return 0;
    }

    uint32_t fourcc = 0;

    char c[4];

    strncpy(c, s.c_str(), 4);

    fourcc = mmioFOURCC(c[0],c[1],c[2],c[3]);

    return fourcc;
}
