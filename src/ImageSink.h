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

#ifndef TCAM_IMAGESINK_H
#define TCAM_IMAGESINK_H

#include "SinkInterface.h"
#include "base_types.h"

#include <functional>
#include <memory>
#include <vector>

/**
 * @addtogroup API
 * @{
 * Main header
 */

namespace tcam
{
   
class ImageSinkBufferPool
{
public:
    explicit ImageSinkBufferPool(const tcam::VideoFormat& format, size_t count)
        : format_(format), buffer_count_to_allocate_(count)
    {
    }

    void set_buffer_collection(std::vector<std::shared_ptr<ImageBuffer>> new_buffers)
    {
        use_external_buffers_ = true;
        buffer_list_ = new_buffers;
    }

    std::vector<std::shared_ptr<ImageBuffer>> get_buffer_collection()
    {
        return buffer_list_;
    }
    void clear()
    {
        use_external_buffers_ = false;
        buffer_list_.clear();
    }
    void create()
    {
        if (buffer_list_.empty() && !use_external_buffers_)
        {
            initialize_internal_buffer();
        }
    }
private:
    void initialize_internal_buffer();

    VideoFormat format_;

    bool use_external_buffers_ = false;
    size_t buffer_count_to_allocate_ = 10;

    std::vector<std::shared_ptr<ImageBuffer>> buffer_list_;
};


class ImageSink : public IImageBufferSink
{
public:
    using image_buffer_cb = std::function<void(const std::shared_ptr<tcam::ImageBuffer>& buffer)>;

public:
    explicit ImageSink(const image_buffer_cb& cb, const tcam::VideoFormat& format, size_t count);

    bool start_stream(std::weak_ptr<IImageBufferPool> requeue_target);
    void stop_stream();

    void push_image(const std::shared_ptr<ImageBuffer>&) final;

    void requeue_buffer(const std::shared_ptr<ImageBuffer>&);

    std::vector<std::shared_ptr<ImageBuffer>> get_buffer_collection();

private:
    void initialize_internal_buffer();

    std::weak_ptr<IImageBufferPool> requeue_pool_;

    image_buffer_cb sh_callback_;

    ImageSinkBufferPool buffer_list_;
};

} /* namespace tcam */

/** @} */

#endif /* TCAM_IMAGESINK_H */
