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

#include "VideoFormat.h"
#include "base_types.h"
#include "Memory.h"

#include <memory>


namespace img
{
struct img_descriptor;
}

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
    /**
     * Creates a ImageBuffer which allocates its own memory according to the VideoFormat passed in.
     * Throws std::bad_alloc when memory could not be allocated.
     */
    explicit ImageBuffer(const VideoFormat& format) noexcept(false);
    /**
     * Creates a ImageBuffer which allocates its own memory according to the VideoFormat passed in.
     * Throws std::bad_alloc when memory could not be allocated.
     * @buffer_size_to_allocate The actual size to allocate. This must be >= format.get_required_buffer_size()
     */
    ImageBuffer(const VideoFormat& format, size_t buffer_size_to_allocate) noexcept(false);
    /**
     * Initializes a ImageBuffer with external memory. Marks the buffer as unowned.
     * @buffer_size The actual size of the buffer pointed to by buffer_ptr. This must be >= format.get_required_buffer_size()
     */
    ImageBuffer(const VideoFormat& format, void* buffer_ptr, size_t buffer_size) noexcept;

    ImageBuffer(const VideoFormat& format, std::shared_ptr<tcam::Memory> buffer) noexcept;

    ImageBuffer() = delete;
    ImageBuffer(const ImageBuffer&) = delete;
    ImageBuffer(ImageBuffer&&) = delete;
    ImageBuffer& operator=(const ImageBuffer&) = delete;
    ImageBuffer& operator=(ImageBuffer&&) = delete;

    ~ImageBuffer();

    static std::shared_ptr<ImageBuffer> make_alloc_buffer(const VideoFormat& fmt,
                                                          size_t actual_buffer_size);

    img::img_descriptor get_img_descriptor() const noexcept;

    /**
     * @return Pointer to actual image data
     */
    void* get_image_buffer_ptr() const noexcept
    {
        return buffer_->ptr();
    }

    /// @name get_image_buffer_size
    /// @brief Get the size of the internal memory
    /// @return size_t - size of the internal memory
    size_t get_image_buffer_size() const noexcept
    {
        return buffer_->length();
    }

    /// @name get_image_size
    /// @brief Get size of the image in bytes
    /// @return size of the image in bytes
    size_t get_valid_data_length() const noexcept
    {
        return valid_data_length_;
    }

    void set_valid_data_length(size_t new_len) noexcept
    {
        valid_data_length_ = new_len;
    }

    tcam_stream_statistics get_statistics() const noexcept
    {
        return statistics_;
    }

    void set_statistics(const tcam_stream_statistics& stats) noexcept
    {
        statistics_ = stats;
    }

    /// @name copy_block
    /// @brief write data to the internal buffer
    /// @param data - pointer to the data that shall be written
    /// @param size - number of bytes that shall be read from data
    /// @param offset - number of bytes at the beginning of the internal buffer that shall be ignored before writing. Default: 0
    /// @return true when data could be written
    bool copy_block(const void* data, size_t size, unsigned int offset) noexcept;

private:
    VideoFormat format_;
    tcam_stream_statistics statistics_ = {};

    size_t valid_data_length_ = 0;
    std::shared_ptr<Memory> buffer_ = nullptr;

    const bool is_own_memory_ = false;
};


} /* namespace tcam*/

/** @} */

#endif /* TCAM_MEMORYBUFFER_H */
