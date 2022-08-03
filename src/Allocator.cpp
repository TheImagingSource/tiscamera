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

#include "Allocator.h"
#include <memory>

namespace
{

    struct DefaultAllocator: public tcam::AllocatorInterface, public std::enable_shared_from_this<DefaultAllocator>
    {

    std::vector<tcam::TCAM_MEMORY_TYPE> get_supported_memory_types() const final
    {
        return {tcam::TCAM_MEMORY_TYPE_USERPTR};
    }

        void* allocate(tcam::TCAM_MEMORY_TYPE t, size_t length, int /*fd*/) final
    {
        if (t != tcam::TCAM_MEMORY_TYPE_USERPTR)
        {
            return nullptr;
        }
        return malloc(length);
    }

    void free(tcam::TCAM_MEMORY_TYPE, void* ptr, size_t, int /*fd*/) final
    {
        if (ptr)
        {
            std::free(ptr);
        }
    }

    std::vector<std::shared_ptr<tcam::Memory>> allocate(size_t buffer_count, tcam::TCAM_MEMORY_TYPE, size_t length, int /*fd*/)
    {
        std::vector<std::shared_ptr<tcam::Memory>> buffer;
        buffer.reserve(buffer_count);
        for (unsigned int i = 0; i < buffer_count; ++i)
        {
            buffer.push_back(std::make_shared<tcam::Memory>(shared_from_this(), tcam::TCAM_MEMORY_TYPE_USERPTR, length));
        }
        return buffer;
    }
    };

} // namespace

std::shared_ptr<tcam::AllocatorInterface> tcam::get_default_allocator()
{
    return std::make_shared<DefaultAllocator>();

}
