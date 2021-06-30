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

#include "internal.h"

#include <cstdlib>
#include <cstring>

using namespace tcam;


ImageBuffer::ImageBuffer(const struct tcam_image_buffer& buf, bool owns_memory)
    : is_own_memory(owns_memory), buffer(buf)
{
}


ImageBuffer::ImageBuffer(const VideoFormat& format, bool owns_memory)
    : is_own_memory(owns_memory), buffer()
{
    buffer.size = format.get_required_buffer_size();
    if (is_own_memory)
    {
        SPDLOG_TRACE("allocating data buffer");
        buffer.pData = (unsigned char*)malloc(buffer.size);
    }
    else
    {
        buffer.pData = nullptr;
    }
    buffer.format = format.get_struct();
    buffer.pitch = format.get_pitch_size();
}


ImageBuffer::~ImageBuffer()
{
    if (is_own_memory)
    {
        if (buffer.pData != nullptr)
        {
            free(buffer.pData);
        }
    }
}


tcam_image_buffer ImageBuffer::getImageBuffer()
{
    return buffer;
}

void ImageBuffer::set_image_buffer(tcam_image_buffer buf)
{
    this->buffer = buf;
}


unsigned char* ImageBuffer::get_data()
{
    return buffer.pData;
}


size_t ImageBuffer::get_buffer_size() const
{
    return buffer.size;
}


size_t ImageBuffer::get_image_size() const
{
    return buffer.length;
}


struct tcam_stream_statistics ImageBuffer::get_statistics() const
{
    return buffer.statistics;
}


bool ImageBuffer::set_statistics(const struct tcam_stream_statistics& stats)
{
    buffer.statistics = stats;
    return true;
}

bool ImageBuffer::set_data(const unsigned char* data, size_t size, unsigned int offset)
{
    if (size + offset > buffer.size)
    {
        return false;
    }

    memcpy(buffer.pData + offset, data, size);

    if (offset == 0)
    {
        buffer.length = size;
    }
    else
    {
        buffer.length = buffer.length + size;
    }

    return true;
}


bool ImageBuffer::lock()
{
    buffer.lock_count++;
    return true;
}


bool ImageBuffer::unlock()
{
    if (buffer.lock_count >= 1)
    {
        buffer.lock_count--;
    }
    return true;
}


bool ImageBuffer::is_locked() const
{
    if (buffer.lock_count == 0)
    {
        return false;
    }
    return true;
}

void ImageBuffer::set_user_data(void* data)
{
    buffer.user_data = data;
}


void* ImageBuffer::get_user_data()
{
    return buffer.user_data;
}


void ImageBuffer::clear()
{
    memset(buffer.pData, 0, buffer.length);
}
