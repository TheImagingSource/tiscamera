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

#include "VideoFormat.h"
#include "logging.h"
#include "utils.h"
#include "format.h"

#include <iomanip>              // setprecision
#include <sstream>
#include <cstring>
#include <cstdlib>

using namespace tcam;


VideoFormat::VideoFormat ()
    :format()
{}


VideoFormat::VideoFormat (const struct tcam_video_format& new_format)
{
    memcpy(&format, &new_format, sizeof(format));
}


VideoFormat::VideoFormat (const VideoFormat& other)
{
    memcpy(&format, &other.format, sizeof(format));
}


VideoFormat& VideoFormat::operator= (const VideoFormat&  other)
{
    memcpy(&format, &other.format, sizeof(format));

    return *this;
}


bool VideoFormat::operator== (const VideoFormat& other) const
{
    return format.fourcc == other.format.fourcc
        && format.width == other.format.width
        && format.height == other.format.height;
        //&& compare_double(format.framerate, other.format.framerate);
}


bool VideoFormat::operator!= (const VideoFormat& other) const
{
    return !(*this == other);
}


struct tcam_video_format VideoFormat::get_struct () const
{
    return format;
}


uint32_t VideoFormat::get_fourcc () const
{
    return format.fourcc;
}


void VideoFormat::set_fourcc (uint32_t fourcc)
{
    format.fourcc = fourcc;
}


double VideoFormat::get_framerate () const
{
    return format.framerate;
}


void VideoFormat::set_framerate (double framerate)
{
    format.framerate = framerate;
}


struct tcam_image_size VideoFormat::get_size () const
{
    tcam_image_size s = {format.width, format.height};
    return s;
}


void VideoFormat::set_size (unsigned int width, unsigned int height)
{
    format.width = width;
    format.height = height;
}


std::string VideoFormat::to_string () const
{
    std::string s;

    s  = "format=";
    s += fourcc2description(format.fourcc);
    s += ",";
    s += "width="     + std::to_string(format.width)   + ",";
    s += "height="    + std::to_string(format.height)  + ",";
    s += "framerate=" + std::to_string(format.framerate);

    return s;
}


bool VideoFormat::from_string (const std::string& desc)
{
    tcam_video_format f = {};

    auto vec = split_string(desc, ",");

    for (auto v : vec)
    {
        auto val = split_string(v, "=");

        if (val.size() != 2)
        {
            tcam_log(TCAM_LOG_ERROR, "Received faulty VideoFormat String \"%s\"", v.c_str());
            return false;
        }

        if (val[0].compare("format") == 0)
        {
            tcam_log(TCAM_LOG_ERROR, "format is  \"%s\"", val[1].c_str());

            f.fourcc  = description2fourcc(val[1].c_str());
        }
        else if (val[0].compare("width") == 0)
        {
            f.width = stoi(val[1]);
        }
        else if (val[0].compare("height") == 0)
        {
            f.height = stoi(val[1]);
        }
        else if (val[0].compare("framerate") == 0)
        {
            f.framerate = stod(val[1]);
        }
        else
        {
            tcam_log(TCAM_LOG_ERROR, "Unknown descriptor in VideoFormat String \"%s\"", val[0].c_str());
            return false;
        }
    }

    this->format = f;

    return true;
}


uint64_t VideoFormat::get_required_buffer_size () const
{
    return get_buffer_length(format.width, format.height, format.fourcc);
}


uint32_t VideoFormat::get_pitch_size () const
{
    return get_pitch_length(format.width, format.fourcc);
}
