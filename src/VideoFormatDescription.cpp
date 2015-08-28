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

#include <algorithm>
#include <cstring>

using namespace tcam;


VideoFormatDescription::VideoFormatDescription (const struct tcam_video_format_description& f,
                                                const std::vector<res_fps>& r)
    : rf(r)
{
    memcpy(&format, &f, sizeof(format));
}


VideoFormatDescription::VideoFormatDescription (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    rf = other.rf;
}


VideoFormatDescription& VideoFormatDescription::operator= (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    rf = other.rf;

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


struct tcam_video_format_description VideoFormatDescription::getStruct () const
{
    return format;
}


uint32_t VideoFormatDescription::getFourcc () const
{
    return format.fourcc;
}


TCAM_FRAMERATE_TYPE VideoFormatDescription::getFramerateType () const
{
    return format.framerate_type;
}


std::vector<res_fps> VideoFormatDescription::getResolutionsFramesrates () const
{
    return rf;
}


std::vector<tcam_image_size> VideoFormatDescription::getResolutions () const
{
    std::vector<tcam_image_size> vec;

    for (auto r : rf)
    {
        vec.push_back(r.resolution);
    }

    return vec;
}


tcam_image_size VideoFormatDescription::getSizeMin () const
{
    return format.min_size;
}


tcam_image_size VideoFormatDescription::getSizeMax () const
{
    return format.max_size;
}


std::vector<double> VideoFormatDescription::getFrameRates (const tcam_image_size& size) const
{
    return getFrameRates(size.width, size.height);
}


std::vector<double> VideoFormatDescription::getFrameRates (unsigned int width, unsigned height) const
{
    std::vector<double> vec;

    if (format.framerate_type == TCAM_FRAMERATE_TYPE_FIXED)
    {
        for (const auto& r :rf)
        {
            if (r.resolution.width == width && r.resolution.height == height)
            {
                vec = r.fps;
            }
        }
    }
    else // TCAM_FRAMERATE_RANGE
    {
        if (format.min_size.width <= width <= format.max_size.width &&
            format.min_size.height <= height <= format.max_size.height)
        {
            // since this is range based we should only have one fps collection
            if (rf.size() == 1)
            {
                vec = rf.at(0).fps;
            }
            {
                tcam_log(TCAM_LOG_ERROR, "Unable to determine correct framerate collection");
            }
        }
    }

    return vec;
}


VideoFormat VideoFormatDescription::createVideoFormat (unsigned int width,
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


bool VideoFormatDescription::isValidVideoFormat (const VideoFormat& to_check) const
{
    auto desc = to_check.getStruct();

    if (format.fourcc != desc.fourcc)
    {
        return false;
    }

    if (!isValidFramerate(to_check.getFramerate()))
    {
        return false;
    }


    if (!isValidResolution(to_check.getSize().width, to_check.getSize().height))
    {
        // tcam_log(TCAM_LOG_ERROR, "Resolution is not");
        return false;
    }

    return true;
}


bool VideoFormatDescription::isValidFramerate (double framerate) const
{
    // auto desc = to_check.get_struct();

    for (const auto& res: rf)
    {
        if (format.framerate_type == TCAM_FRAMERATE_TYPE_FIXED)
        {
            for (const auto& f : res.fps)
            {
                if (compare_double(framerate, f))
                {
                    return true;
                }
            }
        }
        else // range
        {
            if (framerate <= res.fps.at(0) && framerate >= res.fps.at(1))
            {
                return true;
            }
        }
    }

    return false;

}


bool VideoFormatDescription::isValidResolution (unsigned int width, unsigned int height) const
{
    if (format.framerate_type == TCAM_FRAMERATE_TYPE_FIXED)
    {
        return (format.min_size.width == width && format.min_size.height == height);
    }
    else
    {
        return (format.min_size.width <= width &&
                format.min_size.height <= height &&
                format.max_size.width >= width && format.max_size.height >= height);
    }
}
