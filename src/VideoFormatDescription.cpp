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

#include "VideoFormatDescription.h"

#include "logging.h"
#include "utils.h"
#include "FormatHandlerInterface.h"

#include <algorithm>
#include <cstring>

using namespace tcam;


VideoFormatDescription::VideoFormatDescription (std::shared_ptr<FormatHandlerInterface> handler,
                                                const struct tcam_video_format_description& f,
                                                const std::vector<framerate_mapping>& r)
    : res(r), format_handler(handler)
{
    memcpy(&format, &f, sizeof(format));
}


VideoFormatDescription::VideoFormatDescription (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    format_handler = other.format_handler;
    res = other.res;
}


VideoFormatDescription& VideoFormatDescription::operator= (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    format_handler = other.format_handler;
    res = other.res;

    return *this;
}


bool VideoFormatDescription::operator== (const VideoFormatDescription& other) const
{
    // TODO: complete comparison
    return (memcmp(&format, &other.format, sizeof(format)) == 0);
}


bool VideoFormatDescription::operator!= (const VideoFormatDescription& other) const
{
    return !(*this == other);
}


bool VideoFormatDescription::operator== (const struct tcam_video_format_description& other) const
{
    if (are_equal(format, other))
    {
        return true;
    }
    return false;
}


bool VideoFormatDescription::operator!= (const struct tcam_video_format_description& other) const
{
    return !(*this == other);
}


struct tcam_video_format_description VideoFormatDescription::get_struct () const
{
    return format;
}


uint32_t VideoFormatDescription::get_fourcc () const
{
    return format.fourcc;
}


uint32_t VideoFormatDescription::get_binning() const
{
    return format.binning;
}


uint32_t VideoFormatDescription::get_skipping ()const
{
    return format.skipping;
}


std::vector<struct tcam_resolution_description> VideoFormatDescription::get_resolutions () const
{
    std::vector<struct tcam_resolution_description> vec;

    for (const auto& r : res)
    {
        vec.push_back(r.resolution);
    }

    return vec;
}


std::vector<double> VideoFormatDescription::get_frame_rates (const tcam_resolution_description& desc) const
{

    for (const auto& m : res)
    {
        if (are_equal(m.resolution, desc))
        {
            return m.framerates;
        }
    }

    return std::vector<double>();
}


std::vector<double> VideoFormatDescription::get_framerates (const tcam_image_size& s) const
{
    if (auto handler = format_handler.lock())
    {
        return handler->get_framerates(s, format.fourcc);
    }

    for (const auto& r : res)
    {
        if (r.resolution.type == TCAM_RESOLUTION_TYPE_FIXED)
        {
            if (are_equal(s, r.resolution.min_size))
            {
                return r.framerates;
            }
        }
        else
        {
            if (is_smaller(r.resolution.min_size, s) && is_smaller(s, r.resolution.max_size))
            {
                return r.framerates;
            }
        }
    }

    return std::vector<double>();
}


VideoFormat VideoFormatDescription::create_video_format (unsigned int width,
                                                         unsigned int height,
                                                         double framerate) const
{

    // TODO validity check

    tcam_video_format f = {};

    f.fourcc = this->format.fourcc;
    f.width = width;
    f.height = height;
    f.framerate = framerate;

    return VideoFormat(f);
}
