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

#pragma once

#include "base_types.h"


#include <algorithm>
#include <cstdio> // size_t
#include <memory>
#include <vector>


namespace tcam
{

class AllocatorInterface;

//
// Used for memory lifetime management
// Will allocate memory with the given method
// or store given memory
// Will call AllocatorInterface::free upon destruction
//
class Memory
{
private:
    TCAM_MEMORY_TYPE type_;
    void* ptr_ = nullptr;
    size_t length_ = 0;
    bool external_ = false;
    // file descriptor used for DMA
    int fd_ = -1;

    std::shared_ptr<AllocatorInterface> allocator_ = nullptr;

public:

    //
    // params:
    //   alloc: allocator to use for alloc/free, may be nullptr
    //   t: Memory type to use
    //   length: size of the memory block
    //   ptr: Pointer to existing memory, optional
    // throws:
    //   std::runtime_error in case of fatal error
    //
    Memory(std::shared_ptr<AllocatorInterface> alloc,
           TCAM_MEMORY_TYPE t,
           size_t length,
           void* ptr = nullptr);

    // Memory(TCAM_MEMORY_TYPE t, void* ptr, size_t length)
    //     : type_(t), ptr_(ptr), length_(length), external_(true)
    // {}

    ~Memory();

    // prevent copies to ensure underlying memory is correctly handled
    Memory(const Memory&) = delete;
    Memory& operator=(const Memory&) = delete;

    TCAM_MEMORY_TYPE type() const
    {
        return type_;
    };

    // pointer to the memory buffer
    void* ptr()
    {
        return ptr_;
    }

    // size of the memory buffer
    size_t length() const
    {
        return length_;
    };

    // associated file descriptor
    // returns -1 if not used
    int file_descriptor() const
    {
        return fd_;
    }
};


} // namespace tcam
