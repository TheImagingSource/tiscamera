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

#include "base_types.h"

#include "VideoFormat.h"

#include <vector>
#include <memory>

namespace tcam
{

// forward declarations
class FormatHandlerInterface;

struct framerate_mapping
{
    struct tcam_resolution_description resolution;

    std::vector<double> framerates;
};


class VideoFormatDescription
{
public:

    VideoFormatDescription () = delete;

    VideoFormatDescription (std::shared_ptr<FormatHandlerInterface> handler,
                            const struct tcam_video_format_description&,
                            const std::vector<framerate_mapping>&);

    VideoFormatDescription (const VideoFormatDescription&);


    explicit VideoFormatDescription (const struct tcam_video_format_description&);


    VideoFormatDescription& operator= (const VideoFormatDescription&);

    bool operator== (const VideoFormatDescription& other) const;
    bool operator!= (const VideoFormatDescription& other) const;

    bool operator== (const struct tcam_video_format_description& other) const;
    bool operator!= (const struct tcam_video_format_description& other) const;

    /**
     * Returns a struct representation of the format description
     * @return tcam_video_format_description
     */
    struct tcam_video_format_description get_struct () const;

    /**
     * Returns the pixel format used
     * @return uint32 containging the fourcc
     */
    uint32_t get_fourcc () const;

    /**
     * Returns the binning used
     * @return uint32 containging the fourcc
     */
    uint32_t get_binning () const;

    /**
     * Returns the skipping used
     */
    uint32_t get_skipping () const;


    std::vector<struct tcam_resolution_description> get_resolutions () const;

    std::vector<double> get_frame_rates (const tcam_resolution_description& size) const;

    std::vector<double> get_framerates (const tcam_image_size& s) const;


    VideoFormat create_video_format (unsigned int width,
                                     unsigned int height,
                                     double framerate) const;

private:

    tcam_video_format_description format;

    std::vector<framerate_mapping> res;

    std::weak_ptr<FormatHandlerInterface> format_handler;

};

} /* namespace tcam */

#endif /* TCAM_VIDEOFORMATDESCRIPTION_H */
