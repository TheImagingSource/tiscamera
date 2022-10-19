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

#ifndef TCAM_VIDEOFORMATDESCRIPTION_H
#define TCAM_VIDEOFORMATDESCRIPTION_H

#include "VideoFormat.h"
#include "base_types.h"

#include <memory>
#include <vector>

namespace tcam
{

// forward declarations
class FormatHandlerInterface;

struct framerate_mapping
{
    tcam_resolution_description resolution;

    std::vector<double> framerates;
};


struct framerate_info
{
    framerate_info() = default;
    explicit framerate_info(std::vector<double> lst);
    framerate_info(double min, double max) : min_ { min }, max_ { max } {}

    bool is_discrete_list() const noexcept
    {
        return !list_.empty();
    }

    std::vector<double> to_list() const;

    bool empty() const noexcept
    {
        return list_.empty() && max_ == 0.;
    }
    double min() const noexcept
    {
        return min_;
    }
    double max() const noexcept
    {
        return max_;
    }
private:
    std::vector<double> list_;
    double min_ = 0.;
    double max_ = 0.;
};

class VideoFormatDescription
{
public:
    VideoFormatDescription() = delete;

    VideoFormatDescription(std::nullptr_t,
                           const tcam_video_format_description& desc,
                           const std::vector<framerate_mapping>& map)
        : VideoFormatDescription(desc, map)
    {
    }

    VideoFormatDescription(const tcam_video_format_description&,
                           const std::vector<framerate_mapping>&);

    VideoFormatDescription(const VideoFormatDescription&) = default;
    VideoFormatDescription& operator=(const VideoFormatDescription&) = default;

    bool operator==(const VideoFormatDescription& other) const;
    bool operator!=(const VideoFormatDescription& other) const;

    bool is_compatible(const tcam::VideoFormat& format) const;

    /**
     * Returns a struct representation of the format description
     * @return tcam_video_format_description
     */
    std::string get_video_format_description_string() const;

    /**
     * Returns the pixel format used
     * @return uint32 containing the fourcc
     */
    uint32_t get_fourcc() const;

    std::vector<tcam_resolution_description> get_resolutions() const;

    std::vector<double> get_framerates(const VideoFormat& s) const;

private:
    tcam_video_format_description format;

    std::vector<framerate_mapping> res;
};

} /* namespace tcam */

#endif /* TCAM_VIDEOFORMATDESCRIPTION_H */
