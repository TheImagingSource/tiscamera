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


class VideoFormatDescription
{
public:
    VideoFormatDescription() = delete;

    VideoFormatDescription(std::shared_ptr<FormatHandlerInterface> handler,
                           const tcam_video_format_description&,
                           const std::vector<framerate_mapping>&);

    VideoFormatDescription(const VideoFormatDescription&) = default;
    VideoFormatDescription& operator=(const VideoFormatDescription&) = default;

    bool operator==(const VideoFormatDescription& other) const;
    bool operator!=(const VideoFormatDescription& other) const;

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

    // #TODO Christopher: I am not totally sure why there is a difference in this. It seems that these could be merged
    std::vector<double> get_frame_rates(const tcam_resolution_description& size) const;

    std::vector<double> get_framerates(const tcam_image_size& s) const;

private:
    tcam_video_format_description format;

    std::vector<framerate_mapping> res;

    std::weak_ptr<FormatHandlerInterface> format_handler;
};

} /* namespace tcam */

#endif /* TCAM_VIDEOFORMATDESCRIPTION_H */
