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

#include "Allocator.h"
#include "ImageBuffer.h"
#include "base_types.h"
#include "error.h"

#include <memory>
#include <vector>

namespace tcam
{

class BufferPool
{

private:

    TCAM_MEMORY_TYPE memory_type_ = TCAM_MEMORY_TYPE_USERPTR;
    std::shared_ptr<AllocatorInterface> allocator_ = nullptr;

    std::vector<std::shared_ptr<ImageBuffer>> buffer_;

public:
    BufferPool(TCAM_MEMORY_TYPE, std::shared_ptr<AllocatorInterface>);
    ~BufferPool();

    // allocate <buffer_count> buffer that support format
    outcome::result<void> allocate(const VideoFormat& format, size_t buffer_count);
    outcome::result<void> clear();

    std::vector<std::weak_ptr<ImageBuffer>> get_buffer();

    TCAM_MEMORY_TYPE get_memory_type() const
    {
        return memory_type_;
    }

}; // class BufferPool

} // namespace tcam
