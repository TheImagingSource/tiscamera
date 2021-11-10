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

#include "internal.h"

using namespace tcam;


ImageSink::ImageSink()
    : status(TCAM_PIPELINE_UNDEFINED), callback(nullptr), c_back(nullptr), user_data(nullptr),
      last_image_buffer(), external_buffer(false), buffer_number(10), buffers()
{
}


bool ImageSink::set_status(TCAM_PIPELINE_STATUS s)
{
    if (status == s)
    {
        return true;
    }

    status = s;

    if (status == TCAM_PIPELINE_PLAYING)
    {
        if (!external_buffer && buffers.empty())
        {
            if (!initialize_internal_buffer())
            {
                return false;
            }
        }
        SPDLOG_INFO("Pipeline started playing");
    }
    else if (status == TCAM_PIPELINE_STOPPED)
    {
        SPDLOG_INFO("Pipeline stopped playing");
    }

    return true;
}


TCAM_PIPELINE_STATUS ImageSink::get_status() const
{
    return status;
}


bool ImageSink::setVideoFormat(const VideoFormat& new_format)
{
    if (status == TCAM_PIPELINE_PLAYING)
    {
        return false;
    }

    format = new_format;

    return true;
}


VideoFormat ImageSink::getVideoFormat() const
{
    return format;
}


bool ImageSink::registerCallback(shared_callback sc, void* ud)
{
    this->sh_callback = sc;
    this->user_data = ud;

    return true;
}


bool ImageSink::registerCallback(sink_callback sc, void* ud)
{
    this->callback = sc;
    this->user_data = ud;

    return true;
}


bool ImageSink::registerCallback(c_callback cc, void* ud)
{
    this->c_back = cc;
    this->user_data = ud;

    return true;
}


void ImageSink::push_image(std::shared_ptr<ImageBuffer> buffer)
{
    last_image_buffer = buffer->getImageBuffer();
    if (sh_callback)
    {
        this->sh_callback(buffer, user_data);
    }
    if (callback != nullptr)
    {
        this->callback(&*buffer, user_data);
    }
    else if (c_back != nullptr)
    {
        this->c_back(&last_image_buffer, user_data);
    }
}


void ImageSink::requeue_buffer(std::shared_ptr<ImageBuffer> buffer)
{
    if (auto ptr = source_.lock())
    {
        ptr->requeue_buffer(buffer);
    }
    else
    {
        SPDLOG_ERROR("Could not requeue buffer. No Source.");
    }
}


bool ImageSink::set_buffer_number(size_t new_number)
{
    if (status == TCAM_PIPELINE_PLAYING)
    {
        return false;
    }

    if (external_buffer)
    {
        return false;
    }

    buffer_number = new_number;

    return true;
}


bool ImageSink::set_buffer_collection(std::vector<std::shared_ptr<ImageBuffer>> new_buffers)
{
    if (status == TCAM_PIPELINE_PLAYING || status == TCAM_PIPELINE_PAUSED)
    {
        return false;
    }

    buffers = new_buffers;
    buffer_number = buffers.size();
    external_buffer = true;

    return false;
}


std::vector<std::shared_ptr<ImageBuffer>> ImageSink::get_buffer_collection()
{
    if (buffers.empty())
    {
        initialize_internal_buffer();
    }

    return buffers;
}


bool ImageSink::delete_buffer_collection()
{
    if (status == TCAM_PIPELINE_PLAYING || status == TCAM_PIPELINE_PAUSED)
    {
        return false;
    }

    external_buffer = false;

    return false;
}


void ImageSink::set_source(std::weak_ptr<SinkInterface> source)
{
    if (status == TCAM_PIPELINE_PLAYING || status == TCAM_PIPELINE_PAUSED)
    {
        return;
    }

    source_ = source;
}


bool ImageSink::initialize_internal_buffer()
{
    buffers.clear();

    for (unsigned int i = 0; i < this->buffer_number; ++i)
    {
        auto ptr = std::make_shared<ImageBuffer>(format, true);
        this->buffers.push_back(ptr);
    }
    return true;
}


void ImageSink::drop_incomplete_frames(bool drop_them)
{
    if (auto source = source_.lock())
    {
        source->drop_incomplete_frames(drop_them);
    }
    else
    {
        SPDLOG_INFO("Unable to get source object to tell it if imcomplete frames should be locked");
    }
}


bool ImageSink::should_incomplete_frames_be_dropped() const
{
    if (auto source = source_.lock())
    {
        return source->should_incomplete_frames_be_dropped();
    }
    else
    {
        SPDLOG_ERROR("Unable to get source object to query if imcomplete frames should be locked");
        return true;
    }
}
