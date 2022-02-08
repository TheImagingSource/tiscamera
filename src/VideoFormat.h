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

#ifndef TCAM_VIDEOFORMAT_H
#define TCAM_VIDEOFORMAT_H

#include "base_types.h"

#include <string>

namespace img
{
struct img_type;
}

namespace tcam
{

/**
 * @class VideoFormat
 * Description of a specific format
 */
class VideoFormat
{

public:
    VideoFormat() = default;

    explicit VideoFormat(const tcam_video_format&) noexcept;
    VideoFormat(uint32_t fourcc,
                tcam_image_size dim,
                image_scaling scale_factors = {},
                double framerate = 0. ) noexcept;

    VideoFormat(const VideoFormat&) = default;
    VideoFormat& operator=(const VideoFormat&) = default;

    bool operator==(const VideoFormat&) const noexcept;
    bool operator!=(const VideoFormat& other) const noexcept;

    /**
     * Returns a struct representation of the format
     * @return tcam_video_format
     */
    struct tcam_video_format get_struct() const noexcept;

    image_scaling get_scaling() const noexcept;
    void set_scaling(const image_scaling& new_scale) noexcept;

    /**
     * Returns the used pixel format
     * @return uint32 containing the fourcc
     */
    uint32_t get_fourcc() const noexcept;
    std::string get_fourcc_string() const;

    void set_fourcc(uint32_t) noexcept;

    /**
     * @return framerate in frames/second
     */
    double get_framerate() const noexcept;

    void set_framerate(double) noexcept;

    tcam_image_size get_size() const noexcept;

    void set_size(unsigned int width, unsigned int height) noexcept;

    std::string to_string() const;

    /**
     * Description for getRequiredBufferSize.
     * @return size in bytes an image with this format will have
     */
    uint64_t get_required_buffer_size() const noexcept;

    /**
     * Description for getPitchSize.
     * @return the size og an image line
     */
    uint32_t get_pitch_size() const noexcept;

    bool is_empty() const noexcept
    {
        return format.fourcc == 0;
    }
    img::img_type get_img_type() const noexcept;
private:
    tcam_video_format format = {};
};


} /*namespace tcam */

#endif /* TCAM_VIDEOFORMAT_H */
