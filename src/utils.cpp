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

#include "utils.h"

#include "internal.h"

#include <algorithm>
#include <cstring>
#include <sys/ioctl.h>
#include <errno.h>
#include <limits>

#define IOCTL_RETRY 4


using namespace tcam;

std::string tcam::propertyType2String (TCAM_PROPERTY_TYPE type)
{
    switch (type)
    {
        case TCAM_PROPERTY_TYPE_BOOLEAN: return "BOOLEAN";
        case TCAM_PROPERTY_TYPE_INTEGER: return "INTEGER";
        case TCAM_PROPERTY_TYPE_DOUBLE: return "DOUBLE";
        case TCAM_PROPERTY_TYPE_STRING: return "STRING";
        case TCAM_PROPERTY_TYPE_ENUMERATION: return "ENUMERATION";
        case TCAM_PROPERTY_TYPE_BUTTON: return "BUTTON";
        case TCAM_PROPERTY_TYPE_UNKNOWN:
        default:
            return "";
    }
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


std::vector<std::string> tcam::split_string (const std::string& to_split, const std::string &delim)
{
    std::vector<std::string> vec;

    size_t beg = 0;
    size_t end = 0;

    while (end != std::string::npos)
    {
        end = to_split.find_first_of(delim, beg);

        std::string s = to_split.substr(beg, end - beg);

        vec.push_back(s);

        beg = end + delim.size();
    }

    return vec;
}


int tcam::tcam_xioctl (int fd, int request, void *arg)
{
    int ret = 0;
    int tries= IOCTL_RETRY;
    do
    {
        ret = ioctl(fd, request, arg);
        // ret = v4l2_ioctl(fd, request, arg);
    }
    while (ret && tries-- &&
           ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

    if (ret && (tries <= 0))
    {
        tcam_log(TCAM_LOG_ERROR,"ioctl (%i) retried %i times - giving up: %s)\n", request, IOCTL_RETRY, strerror(errno));
    }

    return (ret);

}


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
    {FOURCC_Y16, "Y16"},

    {FOURCC_Y444, "Y444"},
    {FOURCC_Y411, "Y411"},

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

    tcam_log(TCAM_LOG_ERROR, "No string for fourcc %u", fourcc);

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


std::vector<double> tcam::create_steps_for_range (double min, double max)
{
    std::vector<double> vec;

    if (max <= min)
        return vec;

    vec.push_back(min);

    // we do not want every framerate to have unnecessary decimals
    // e.g. 1.345678 instead of 1.00000
    double current_step = (int)min;

    // 0.0 is not a valid framerate
    if (current_step < 1.0)
        current_step = 1.0;

    while (current_step < max)
    {
        vec.push_back(current_step);

        if (current_step < 20.0)
        {
            current_step += 1;
        }
        else
        {
            current_step += 5.0;
        }
    }

    if (vec.back() != max)
    {
        vec.push_back(max);
    }
    return vec;
}


uint64_t tcam::get_buffer_length (unsigned int width, unsigned int height, uint32_t fourcc)
{
    if (width == 0 || height == 0 || fourcc == 0)
    {
        return 0;
    }

    uint64_t size = width * height * (img::get_bits_per_pixel(fourcc) / 8);

    return size;
}


unsigned int tcam::tcam_get_required_buffer_size (const struct tcam_video_format* format)
{
    if (format == nullptr)
    {
        return 0;
    }

    return get_buffer_length(format->width, format->height, format->fourcc);
}

uint32_t tcam::get_pitch_length (unsigned int width, uint32_t fourcc)
{
    if (width == 0 || fourcc == 0)
    {
        return 0;
    }

    return width * (img::get_bits_per_pixel(fourcc) / 8);
}


tcam_image_size tcam::calculate_auto_center (const tcam_image_size& sensor, const tcam_image_size& image)
{
    tcam_image_size ret = {};

    if (image.width > sensor.width || image.height > sensor.height)
    {
        return ret;
    }

    ret.width = (sensor.width / 2) - (image.width /2);
    ret.height = (sensor.height / 2) - (image.height / 2);

    return ret;
}


std::shared_ptr<Property> tcam::find_property (std::vector<std::shared_ptr<Property>>& properties,
                                               TCAM_PROPERTY_ID property_id)
{
    for (auto& p : properties)
    {
        if (p->get_ID() == property_id)
        {
            return p;
        }
    }

    return nullptr;
}


std::shared_ptr<Property> tcam::find_property (std::vector<std::shared_ptr<Property>>& properties,
                                               const std::string& property_name)
{

    auto f = [&property_name] (const std::shared_ptr<Property>& p)
        {
            if (p->get_name().compare(property_name) == 0)
                return true;
            return false;
        };

    auto iter = std::find_if(properties.begin(), properties.end(), f);

    if (iter != properties.end())
    {
        return *iter;
    }

    return nullptr;
}


bool tcam::compare_double (double val1, double val2)
{
    return std::fabs(val1 - val2) < std::numeric_limits<double>::epsilon();
}


bool tcam::are_equal (const tcam_image_size& s1,
                      const tcam_image_size& s2)
{
    if (s1.height == s2.height
        && s1.width == s2.width)
    {
        return true;
    }
    return false;
}


bool tcam::are_equal (const struct tcam_resolution_description& res1,
                      const struct tcam_resolution_description& res2)
{
    if (res1.type == res2.type
        && res1.framerate_count == res2.framerate_count
        && are_equal(res1.max_size, res2.max_size)
        && are_equal(res1.min_size,res2.min_size))
    {
        return true;
    }

    return false;
}


bool tcam::are_equal (const struct tcam_video_format_description& fmt1,
                      const struct tcam_video_format_description& fmt2)
{
    if (fmt1.fourcc == fmt2.fourcc
        && fmt1.binning == fmt2.binning
        && fmt1.skipping == fmt2.skipping
        && fmt1.resolution_count == fmt2.resolution_count
        && strcmp(fmt1.description, fmt2.description) == 0)
    {
        return true;
    }

    return false;
}


TCAM_PROPERTY_ID tcam::generate_unique_property_id ()
{
    static unsigned int id_to_use;
    static unsigned int id_prefix = 0x199f0000;

    TCAM_PROPERTY_ID new_id = id_prefix ^ id_to_use;
    id_to_use++;
    return new_id;
}
