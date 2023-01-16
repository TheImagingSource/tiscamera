/*
 * Copyright 2022 The Imaging Source Europe GmbH
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

#include "AravisAllocator.h"


void* tcam::aravis::AravisAllocator::allocate(TCAM_MEMORY_TYPE t, size_t length, int /*fd*/)
{
    if (t != tcam::TCAM_MEMORY_TYPE_USERPTR)
    {
        return nullptr;
    }

    return std::malloc(length);
}


void tcam::aravis::AravisAllocator::free(TCAM_MEMORY_TYPE, void* ptr, size_t, int /*fd*/)
{
    std::free(ptr);
}


std::vector<std::shared_ptr<tcam::Memory>> tcam::aravis::AravisAllocator::allocate(size_t buffer_count,
                                                                     TCAM_MEMORY_TYPE t,
                                                                     size_t length,
                                                                     int /*fd*/)
{
    if (t != TCAM_MEMORY_TYPE_USERPTR)
    {
        return {};
    }

     std::vector<std::shared_ptr<tcam::Memory>> buffer;
     buffer.reserve(length);

     for (unsigned int i = 0; i < buffer_count; ++i)
     {
        buffer.push_back(std::make_shared<tcam::Memory>(shared_from_this(), TCAM_MEMORY_TYPE_USERPTR, length));
     }

    return buffer;
}
