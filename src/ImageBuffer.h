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

#ifndef TCAM_MEMORYBUFFER_H
#define TCAM_MEMORYBUFFER_H

#include "base_types.h"

#include "VideoFormat.h"

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

/// @class ImageBuffer
/// @brief Transport class for memory, format and statistics representing an actual image
class ImageBuffer
{

public:

    // will not take ownership of memory given in tcam_image_buffer
    explicit ImageBuffer (const struct tcam_image_buffer&, bool owns_memory=false);

    // will allocate buffer memory
    explicit ImageBuffer (const VideoFormat&, bool owns_memory=false);

    ImageBuffer () = delete;

    ~ImageBuffer ();

    /**
     *
     */
    tcam_image_buffer getImageBuffer ();

    void set_image_buffer (tcam_image_buffer);

    /**
     * @return Pointer to actual image data
     */
    unsigned char* get_data ();

    /// @name get_buffer_size
    /// @brief Get the size of the internal memory
    /// @return size_t - size of the internal memory
    size_t get_buffer_size () const;

    /// @name get_image_size
    /// @brief Get size of the image in bytes
    /// @return size of the image in bytes
    size_t get_image_size () const;


    struct tcam_stream_statistics get_statistics () const;

    bool set_statistics (const struct tcam_stream_statistics&);

    /// @name set_data
    /// @brief write data to the internal buffer
    /// @param data - pointer to the data that shall be written
    /// @param size - number of bytes that shall be read from data
    /// @param offset - number of bytes at the beginning of the internal buffer that shall be ignored before writing. Default: 0
    /// @return true when data could be written
    bool set_data (const unsigned char* data, size_t size, unsigned int offset = 0);

    bool lock ();

    bool unlock ();

    bool is_locked () const;

    bool is_complete () const;

    void set_user_data (void* data);

    void* get_user_data ();

    /// @brief Fills MemoryBuffer with 0
    void clear ();

private:

    const bool is_own_memory;
    struct tcam_image_buffer buffer;

};


} /* namespace tcam*/

/** @} */

#endif /* TCAM_MEMORYBUFFER_H */
