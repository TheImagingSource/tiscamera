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

#include "Memory.h"

#include "Allocator.h"

#include <stdexcept>


tcam::Memory::Memory(std::shared_ptr<AllocatorInterface> alloc,
                     TCAM_MEMORY_TYPE t,
                     size_t length,
                     void* ptr)
    : type_(t), ptr_(ptr), length_(length), allocator_(alloc)
{
    auto types = allocator_->get_supported_memory_types();
    if (std::find(types.begin(), types.end(), t) == types.end())
    {
        throw std::runtime_error("Memory type not supported");
    }

    if (!ptr)
    {
        ptr_ = allocator_->allocate(type_, length_);

        if (!ptr_)
        {
            throw std::runtime_error("Unable to allocate memory");
        }
    }
}


tcam::Memory::~Memory()
{
    if (ptr_ && !external_)
    {
        allocator_->free(type_, ptr_, length_);
        ptr_ = nullptr;
        length_ = 0;
    }
}
