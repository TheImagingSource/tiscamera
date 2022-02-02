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

#include "FormatHandlerInterface.h"
#include "logging.h"
#include "utils.h"

#include <algorithm>
#include <cstring>

using namespace tcam;


VideoFormatDescription::VideoFormatDescription(std::shared_ptr<FormatHandlerInterface> handler,
                                               const struct tcam_video_format_description& f,
                                               const std::vector<framerate_mapping>& r)
    : format(f), res(r), format_handler(handler)
{
}

bool VideoFormatDescription::operator==(const VideoFormatDescription& other) const
{
    // TODO: complete comparison
    return format == other.format;
}

bool VideoFormatDescription::operator!=(const VideoFormatDescription& other) const
{
    return !(*this == other);
}

uint32_t VideoFormatDescription::get_fourcc() const
{
    return format.fourcc;
}

std::string VideoFormatDescription::get_video_format_description_string() const
{
    return format.description;
}


std::vector<struct tcam_resolution_description> VideoFormatDescription::get_resolutions() const
{
    std::vector<struct tcam_resolution_description> vec;

    for (const auto& r : res) { vec.push_back(r.resolution); }

    return vec;
}


std::vector<double> VideoFormatDescription::get_frame_rates(
    const tcam_resolution_description& desc) const
{
    for (const auto& m : res)
    {
        if (m.resolution == desc)
        {
            return m.framerates;
        }
    }
    return {};
}


std::vector<double> VideoFormatDescription::get_framerates(const tcam_image_size& s) const
{
    if (auto handler = format_handler.lock())
    {
        return handler->get_framerates(s, format.fourcc);
    }

    for (const auto& r : res)
    {
        if (r.resolution.type == TCAM_RESOLUTION_TYPE_FIXED)
        {
            if (s == r.resolution.min_size)
            {
                return r.framerates;
            }
        }
        else
        {
            if ((r.resolution.min_size < s) && (s < r.resolution.max_size))
            {
                return r.framerates;
            }
        }
    }

    return std::vector<double>();
}