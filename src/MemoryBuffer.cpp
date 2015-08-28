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

#include <cstring>
#include <cstdlib>

using namespace tcam;


MemoryBuffer::MemoryBuffer (const struct tcam_image_buffer& buf)
    : buffer(buf), lock_count(0)
{}


MemoryBuffer::~MemoryBuffer ()
{}


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
    lock_count = 1;
    return true;
}


bool MemoryBuffer::unlock ()
{
    lock_count = 0;
    return true;
}


bool MemoryBuffer::is_locked () const
{
    if (lock_count == 0)
    {
        return false;
    }
    return true;
}


void MemoryBuffer::clear ()
{
    memset(buffer.pData, 0, buffer.length);
}
