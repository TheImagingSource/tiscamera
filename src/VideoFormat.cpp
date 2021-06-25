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

#include <dutils_img/fcc_to_string.h>   // img::fcc_to_string
#include <dutils_img/image_fourcc_func.h>

#include <cstdlib>
#include <cstring>

using namespace tcam;


VideoFormat::VideoFormat(const tcam_video_format& new_format)
{
    format = new_format;
}

bool VideoFormat::operator==(const VideoFormat& other) const
{
    return format.fourcc == other.format.fourcc && format.width == other.format.width
           && format.height == other.format.height;
    //&& compare_double(format.framerate, other.format.framerate);
}


bool VideoFormat::operator!=(const VideoFormat& other) const
{
    return !(*this == other);
}


tcam_video_format VideoFormat::get_struct() const
{
    return format;
}

uint32_t VideoFormat::get_fourcc() const
{
    return format.fourcc;
}

void VideoFormat::set_fourcc(uint32_t fourcc)
{
    format.fourcc = fourcc;
}


double VideoFormat::get_framerate() const
{
    return format.framerate;
}


void VideoFormat::set_framerate(double framerate)
{
    format.framerate = framerate;
}


tcam_image_size VideoFormat::get_size() const
{
    tcam_image_size s = { format.width, format.height };
    return s;
}


void VideoFormat::set_size(unsigned int width, unsigned int height)
{
    format.width = width;
    format.height = height;
}


std::string VideoFormat::to_string() const
{
    std::string s;

    s = "format=";
    s += get_fourcc_string();
    s += ",";
    s += "width=" + std::to_string(format.width) + ",";
    s += "height=" + std::to_string(format.height) + ",";
    s += "framerate=" + std::to_string(format.framerate);

    return s;
}

uint64_t VideoFormat::get_required_buffer_size() const
{
    return img::calc_minimum_img_size( (img::fourcc)format.fourcc, { (int)format.width, (int)format.height } );
}


uint32_t VideoFormat::get_pitch_size() const
{
    return img::calc_minimum_pitch( (img::fourcc)format.fourcc, format.width );
}

std::string VideoFormat::get_fourcc_string() const
{
    return img::fcc_to_string( get_fourcc() );
}
