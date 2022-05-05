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

#include "../Allocator.h"

#include <memory>

namespace tcam
{
namespace aravis
{
class AravisAllocator : public AllocatorInterface, public std::enable_shared_from_this<AravisAllocator>
{
public:

    AravisAllocator(){};

    std::vector<TCAM_MEMORY_TYPE> get_supported_memory_types() const final
    {
        return {TCAM_MEMORY_TYPE_USERPTR};
    };

    void* allocate(TCAM_MEMORY_TYPE, size_t, int fd = 0) final;
    void free(TCAM_MEMORY_TYPE, void* ptr, size_t, int fd = 0) final;

    std::vector<std::shared_ptr<Memory>> allocate(
        size_t buffer_count, TCAM_MEMORY_TYPE, size_t, int fd = 0) final;
};
}

} // namespace tcam::aravis
