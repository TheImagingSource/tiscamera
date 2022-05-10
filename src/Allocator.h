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

#include "ImageBuffer.h"
#include "base_types.h"
//#include "Memory.h"

#include <vector>
#include <cstdio> // size_t
#include <algorithm>
#include <memory>


namespace tcam
{

class Memory;

class AllocatorInterface
{
public:
    virtual ~AllocatorInterface() {};

    virtual std::vector<TCAM_MEMORY_TYPE> get_supported_memory_types() const = 0;

    virtual void* allocate(TCAM_MEMORY_TYPE, size_t, int fd=0) = 0;
    virtual void free(TCAM_MEMORY_TYPE, void* ptr, size_t, int fd = 0) = 0;

    virtual std::vector<std::shared_ptr<Memory>> allocate(size_t buffer_count, TCAM_MEMORY_TYPE, size_t, int fd=0) = 0;

};

std::shared_ptr<AllocatorInterface> get_default_allocator();

} // namespace tcam
