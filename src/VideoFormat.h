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

namespace tcam
{

/**
 * @class VideoFormat
 * Description of a specific format
 */
class VideoFormat
{

public:

    VideoFormat ();

    explicit VideoFormat (const struct tcam_video_format&);

    VideoFormat (const VideoFormat&);

    VideoFormat& operator= (const VideoFormat&);

    bool operator== (const VideoFormat&) const;

    bool operator!= (const VideoFormat& other) const;


    /**
     * Returns a struct representation of the format
     * @return tcam_video_format
     */
    struct tcam_video_format get_struct () const;

    /**
     * Returns the used pixel format
     * @return uint32 containing the fourcc
     */
    uint32_t get_fourcc () const;

    void set_fourcc (uint32_t);

    /**
     * @return framerate in frames/second
     */
    double get_framerate () const;

    void set_framerate (double);

    struct tcam_image_size get_size () const;

    void set_size (unsigned int width, unsigned int height);

    std::string to_string () const;

    bool from_string (const std::string&);

    /**
     * Description for getRequiredBufferSize.
     * @return size in bytes an image with this format will have
     */
    uint64_t get_required_buffer_size () const;

    /**
     * Description for getPitchSize.
     * @return the size og an image line
     */
    uint32_t get_pitch_size () const;

private:

    struct tcam_video_format format;

};


} /*namespace tcam */

#endif /* TCAM_VIDEOFORMAT_H */
