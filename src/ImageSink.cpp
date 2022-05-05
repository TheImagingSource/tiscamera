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

#include "ImageSink.h"

#include "logging.h"

#include <cassert>

using namespace tcam;

ImageSink::ImageSink(const image_buffer_cb& cb, const tcam::VideoFormat& format, size_t count)
    : sh_callback_(cb), buffer_list_(format, count)
{
    assert(cb != nullptr);
}

bool ImageSink::start_stream(std::weak_ptr<IImageBufferPool> requeue_target)
{
    buffer_list_.create();
    requeue_pool_ = requeue_target;
    return true;
}

void ImageSink::stop_stream()
{
    requeue_pool_.reset();
}

void ImageSink::push_image(const std::shared_ptr<ImageBuffer>& buffer)
{
    sh_callback_(buffer);
}

void ImageSink::requeue_buffer(const std::shared_ptr<ImageBuffer>& buffer)
{
    if (auto ptr = requeue_pool_.lock())
    {
        ptr->requeue_buffer(buffer);
    }
    else
    {
        SPDLOG_ERROR("Could not requeue buffer. No Source.");
    }
}

std::vector<std::shared_ptr<ImageBuffer>> ImageSink::get_buffer_collection()
{
    return buffer_list_.get_buffer_collection();
}

void ImageSinkBufferPool::initialize_internal_buffer()
{
    buffer_list_.clear();
    return;

    for (size_t i = 0; i < this->buffer_count_to_allocate_; ++i)
    {
        auto ptr = std::make_shared<ImageBuffer>(format_);
        this->buffer_list_.push_back(ptr);
    }
}