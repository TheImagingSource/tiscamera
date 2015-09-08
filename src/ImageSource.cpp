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

#include "ImageSource.h"

#include "internal.h"

using namespace tcam;

ImageSource::ImageSource ()
    : current_status(TCAM_PIPELINE_UNDEFINED), n_buffers(10)
{}


ImageSource::~ImageSource ()
{
    device->release_buffers();
}


bool ImageSource::set_status (TCAM_PIPELINE_STATUS status)
{
    current_status = status;

    if (current_status == TCAM_PIPELINE_PLAYING)
    {
        this->initialize_buffers();

        device->initialize_buffers(buffers);
        device->set_sink(shared_from_this());

        stream_start = std::chrono::steady_clock::now();

        if ( device->start_stream())
        {
            tcam_log(TCAM_LOG_DEBUG, "PLAYING....");
        }
        else
        {
            tcam_log(TCAM_LOG_ERROR, "Unable to start stream from device.");
            return false;
        }

    }
    else if (current_status == TCAM_PIPELINE_STOPPED)
    {
        tcam_log(TCAM_LOG_INFO, "Source changed to state STOPPED");
        device->stop_stream();
        device->release_buffers();
    }


    return true;
}


TCAM_PIPELINE_STATUS ImageSource::get_status () const
{
    return current_status;
}


bool ImageSource::setDevice (std::shared_ptr<DeviceInterface> dev)
{
    //tcam_log(TCAM_LOG_DEBUG, "Received device to use as source.");

    if (current_status == TCAM_PIPELINE_PAUSED || current_status == TCAM_PIPELINE_PLAYING)
    {
        return false;
    }

    device = dev;

    return true;
}


bool ImageSource::setVideoFormat (const VideoFormat& f)
{
    return device->set_video_format(f);
}


VideoFormat ImageSource::getVideoFormat () const
{
    return device->get_active_video_format();
}


void ImageSource::push_image (std::shared_ptr<MemoryBuffer> buffer)
{
    auto stats = buffer->get_statistics();
    auto end = std::chrono::steady_clock::now();
    auto elapsed = end - stream_start;

    if (stats.frame_count > 0 && std::chrono::duration_cast<std::chrono::seconds>(elapsed).count())
    {
        stats.framerate = (double)stats.frame_count / (double)std::chrono::duration_cast<std::chrono::seconds>(elapsed).count();
    }
    buffer->set_statistics(stats);

    if (!pipeline.expired())
        pipeline.lock()->push_image(buffer);
    else
        tcam_log(TCAM_LOG_ERROR, "Pipeline over expiration date.");
}


void ImageSource::initialize_buffers ()
{
    device->release_buffers();
    buffers.clear();

    VideoFormat f = device->get_active_video_format();

    struct tcam_video_format format = f.get_struct();
    int bit_depth = img::get_bits_per_pixel(format.fourcc);

    for (unsigned int i = 0; i < this->n_buffers; ++i)
    {
        struct tcam_image_buffer b = {};

        b.pData = NULL;
        b.length = format.width * format.height * bit_depth;
        b.format = format;
        b.pitch = format.width * bit_depth / 8;

        auto ptr = std::make_shared<MemoryBuffer>(MemoryBuffer(b));

        this->buffers.push_back(ptr);
    }
}


bool ImageSource::setSink (std::shared_ptr<SinkInterface> sink)
{
    this->pipeline = sink;

    return true;
}
