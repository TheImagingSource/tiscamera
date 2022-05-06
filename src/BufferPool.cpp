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


#include "BufferPool.h"

#include <algorithm>

#include "logging.h"


tcam::BufferPool::BufferPool(TCAM_MEMORY_TYPE t, std::shared_ptr<AllocatorInterface> ai)
    : memory_type_(t), allocator_(ai)
{
    auto allo_mem_types = allocator_->get_supported_memory_types();
    if (std::find(allo_mem_types.begin(), allo_mem_types.end(), t) == allo_mem_types.end())
    {
        throw std::runtime_error("Memory type not supported");
    }
}


tcam::BufferPool::~BufferPool() {}


outcome::result<void> tcam::BufferPool::configure(const VideoFormat& format,
                                                  size_t buffer_count)
{
    auto ret = clear();

    format_ = format;
    count_ = buffer_count;

    return outcome::success();
}

outcome::result<void> tcam::BufferPool::allocate(const VideoFormat& format,
                                                 size_t buffer_count)
{

    auto memory = allocator_->allocate(buffer_count, memory_type_, format.get_required_buffer_size());

    if (memory.size() != buffer_count)
    {
        SPDLOG_ERROR("Could only allocate {} of {} requested buffer", memory.size(), buffer_count);
        return status::UndefinedError;
    }

    buffer_.clear();
    buffer_.reserve(memory.size());

    for (auto& m : memory)
    {
        buffer_.push_back(std::make_shared<ImageBuffer>(format, m));
    }

    return outcome::success();
}

outcome::result<void> tcam::BufferPool::allocate()
{

    auto memory = allocator_->allocate(count_, memory_type_, format_.get_required_buffer_size());

    if (memory.size() != count_)
    {
        SPDLOG_ERROR("Could only allocate {} of {} requested buffer", memory.size(), count_);
        return status::UndefinedError;
    }

    buffer_.clear();
    buffer_.reserve(memory.size());

    for (auto& m : memory)
    {
        buffer_.push_back(std::make_shared<ImageBuffer>(format_, m));
    }

    return outcome::success();
}


outcome::result<void> tcam::BufferPool::clear()
{
    buffer_.clear();

    return outcome::success();
}

std::vector<std::weak_ptr<tcam::ImageBuffer>> tcam::BufferPool::get_buffer()
{
    std::vector<std::weak_ptr<tcam::ImageBuffer>> ret;

    ret.reserve(buffer_.size());

    for (auto& buf : buffer_) { ret.push_back(buf); }

    return ret;
}
