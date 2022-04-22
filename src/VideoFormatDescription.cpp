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


VideoFormatDescription::VideoFormatDescription(const tcam_video_format_description& f,
                                               const std::vector<framerate_mapping>& r)
    : format(f), res(r)
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


bool VideoFormatDescription::is_compatible(const tcam::VideoFormat& format) const
{
    if (format.get_fourcc() == get_fourcc())
    {
        auto fps = get_framerates(format);
        if (fps.empty())
        {
            return false;
        }
        if (std::any_of(fps.begin(), fps.end(), [&format](const double& d)
        {
            return d == format.get_framerate();
        }))
        {
            return true;
        }

        return false;
    }
    return false;
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

std::vector<double> VideoFormatDescription::get_framerates(const VideoFormat& fmt) const
{
    if (fmt.get_fourcc() != format.fourcc)
    {
        return {};
    }

    auto s = fmt.get_size();
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
            if (tcam::is_inside_dim_range(r.resolution.min_size, r.resolution.max_size, s))
            {
                return r.framerates;
            }
        }
    }

    return {};
}


framerate_info::framerate_info(std::vector<double> lst) : list_ { std::move(lst) }
{
    assert(!list_.empty());
    std::sort(list_.begin(), list_.end());
    min_ = list_.front();
    max_ = list_.back();
}

std::vector<double> framerate_info::to_list() const
{
    if (is_discrete_list())
    {
        return list_;
    }
    return tcam::create_steps_for_range(min_, max_);
}
