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

#include "ImageBuffer.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <dutils_img/image_transform_base.h>

#include "logging.h"

using namespace tcam;

ImageBuffer::ImageBuffer(const VideoFormat& format)
    : ImageBuffer(format, format.get_required_buffer_size())
{
}

ImageBuffer::ImageBuffer(const VideoFormat& format, size_t buffer_size_to_allocate)
    : format_(format),
     //buffer_size_(buffer_size_to_allocate),
      is_own_memory_(true)
{
    assert(buffer_size_to_allocate >= format.get_required_buffer_size());
    SPDLOG_ERROR("NO Memory");
    // buffer_ptr_ = malloc(buffer_size_);
    // if (buffer_ptr_ == nullptr)
    // {
    //     throw std::bad_alloc();
    // }
}

ImageBuffer::ImageBuffer(const VideoFormat& format, void* /*buffer_ptr*/, size_t /*buffer_size*/) noexcept
    : format_(format),
    //, buffer_size_(buffer_size), buffer_ptr_(buffer_ptr),
      is_own_memory_(false)
{
    //assert(buffer_size >= format.get_required_buffer_size());
}

ImageBuffer::ImageBuffer(const VideoFormat& format, std::shared_ptr<Memory> buffer) noexcept
    : format_(format), buffer_(buffer)
{

}


ImageBuffer::~ImageBuffer()
{
    if (is_own_memory_)
    {
//        free(buffer_ptr_);
    }
}

bool ImageBuffer::copy_block(const void* data, size_t size, unsigned int offset) noexcept
{
    if (size + offset > buffer_->length())
    {
        return false;
    }

    memcpy(static_cast<char*>(buffer_->ptr()) + offset, data, size);

    if (offset == 0)
    {
        valid_data_length_ = size;
    }
    else
    {
        valid_data_length_ += size;
    }
    return true;
}

std::shared_ptr<tcam::ImageBuffer> tcam::ImageBuffer::make_alloc_buffer(const VideoFormat& fmt,
                                                                        size_t actual_buffer_size)
{
    return std::make_shared<ImageBuffer>(fmt, actual_buffer_size);
}

img::img_descriptor ImageBuffer::get_img_descriptor() const noexcept
{
    return img::make_img_desc_from_linear_memory(format_.get_img_type(), get_image_buffer_ptr());
}
