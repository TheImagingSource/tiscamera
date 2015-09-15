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

#include "MemoryBuffer.h"

#include "internal.h"

#include <cstring>
#include <cstdlib>

using namespace tcam;


MemoryBuffer::MemoryBuffer (const struct tcam_image_buffer& buf)
    : is_own_memory(false), buffer(buf)
{}


MemoryBuffer::MemoryBuffer (const VideoFormat& format)
    : is_own_memory(true), buffer()
{

    buffer.length = format.get_required_buffer_size();
    buffer.pData = (unsigned char*)malloc(buffer.length);
    buffer.format = format.get_struct();
    buffer.pitch = format.get_pitch_size();

    tcam_log(TCAM_LOG_DEBUG, "Adress is %p", buffer.pData);
}


MemoryBuffer::~MemoryBuffer ()
{
    if (is_own_memory)
    {
        if (buffer.pData != nullptr)
        {
            free(buffer.pData);
        }
    }
}


tcam_image_buffer MemoryBuffer::getImageBuffer ()
{
    return buffer;
}

void MemoryBuffer::set_image_buffer (tcam_image_buffer buf)
{
    this->buffer = buf;
}


unsigned char* MemoryBuffer::get_data ()
{
    return buffer.pData;
}


struct tcam_stream_statistics MemoryBuffer::get_statistics () const
{
    return buffer.statistics;
}


bool MemoryBuffer::set_statistics (const struct tcam_stream_statistics& stats)
{
    buffer.statistics = stats;;

    return true;
}


bool MemoryBuffer::lock ()
{
    buffer.lock_count++;
    return true;
}


bool MemoryBuffer::unlock ()
{
    if (buffer.lock_count >=1)
    {
        buffer.lock_count--;
    }
    return true;
}


bool MemoryBuffer::is_locked () const
{
    if (buffer.lock_count == 0)
    {
        return false;
    }
    return true;
}


void MemoryBuffer::clear ()
{
    memset(buffer.pData, 0, buffer.length);
}
