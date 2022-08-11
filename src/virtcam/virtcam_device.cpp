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

#include "virtcam_device.h"

#include "../logging.h"
#include "../utils.h"
#include "dutils_img/image_fourcc.h"
#include "dutils_img/image_fourcc_enum.h"
#include "dutils_img/image_fourcc_func.h"
#include "spdlog/spdlog.h"

#include <algorithm>
#include <bits/stdc++.h>
#include <cmath> /* ceil */
#include <cstdint>
#include <cstring> /* memcpy*/
#include <dutils_img/image_bayer_pattern.h>
#include <dutils_img/image_transform_base.h>
#include <dutils_img/pixel_structs.h>
#include <sys/mman.h>

#include "virtcam_generator.h"

using namespace img::by_transform;


tcam::virtcam::VirtcamDevice::VirtcamDevice(const DeviceInfo& info)
{
    device = info;

    tcam_resolution_description res_type = {
        TCAM_RESOLUTION_TYPE_RANGE,
        { 640, 480 },
        { 1920, 1080 },
        16, 4,
        image_scaling {},
    };
    framerate_mapping m = { res_type, { 15., 30., 60.} };

    for (auto&& d : get_supported_fourcc())
    {
        tcam_video_format_description desc = { (uint32_t)d, "" };
        available_videoformats_.push_back(tcam::VideoFormatDescription(nullptr, desc, { m }));
    }

    generate_properties();
}

tcam::virtcam::VirtcamDevice::VirtcamDevice(const DeviceInfo& info, const std::vector<tcam::VideoFormatDescription>& desc)
{
    device = info;
    available_videoformats_ = desc;
}

tcam::virtcam::VirtcamDevice::~VirtcamDevice()
{
    stop_stream();
}

tcam::DeviceInfo tcam::virtcam::VirtcamDevice::get_device_description() const
{
    return device;
}

bool tcam::virtcam::VirtcamDevice::set_video_format(const VideoFormat& fmt)
{
    active_video_format_ = fmt;
    return true;
}

tcam::VideoFormat tcam::virtcam::VirtcamDevice::get_active_video_format() const
{
    return active_video_format_;
}

std::vector<tcam::VideoFormatDescription> tcam::virtcam::VirtcamDevice::
    get_available_video_formats()
{
    return available_videoformats_;
}

bool tcam::virtcam::VirtcamDevice::initialize_buffers(std::shared_ptr<BufferPool> pool)
{
    pool_ = pool;

    auto b = pool_->get_buffer();

    buffer_queue_.clear();
    buffer_queue_.reserve(b.size());

    for (auto& weak_buffer : b)
    {
        if (auto buf = weak_buffer.lock())
        {
            buffer_queue_.push_back(buf);
        }
    }

    return true;
}

bool tcam::virtcam::VirtcamDevice::release_buffers()
{
    buffer_queue_.clear();
    return true;
}

void tcam::virtcam::VirtcamDevice::requeue_buffer(const std::shared_ptr<ImageBuffer>& buf)
{
    std::scoped_lock lck { buffer_queue_mutex_ };
    buffer_queue_.push_back(buf);
}

bool tcam::virtcam::VirtcamDevice::start_stream(const std::shared_ptr<IImageBufferSink>& sink)
{
    stop_stream();
    generator_ = tcam::virtcam::get_generator(static_cast<img::fourcc>(active_video_format_.get_fourcc()));

    stream_sink_ = sink;

    stream_thread_ended_ = false;
    stream_thread_ = std::thread([this] { stream_thread_main(); });

    start_time_ = std::chrono::high_resolution_clock::now();

    return true;
}

void tcam::virtcam::VirtcamDevice::stop_stream()
{
    if (!stream_thread_.joinable())
        return;

    {
        std::scoped_lock lck { stream_thread_mutex_ };
        stream_thread_ended_ = true;
        stream_thread_cv_.notify_all();
    }

    stream_thread_.join();
}

void tcam::virtcam::VirtcamDevice::trigger_device_lost ()
{
    stop_stream();
    notify_device_lost();
}


void tcam::virtcam::VirtcamDevice::stream_thread_main()
{
    const int64_t timeout_in_us = 1'000'000 / active_video_format_.get_framerate();

    const auto send_interval = std::chrono::microseconds(timeout_in_us);

    std::chrono::steady_clock::time_point next_time = std::chrono::steady_clock::now();

    // SPDLOG_ERROR("Start thread");

    while (true)
    {
        bool send_image = false;

        {
            std::unique_lock lck { stream_thread_mutex_ };

            if (!stream_thread_ended_)
            {
                auto res = stream_thread_cv_.wait_until(lck, next_time);
                send_image = res == std::cv_status::timeout;
                // trigger mode but no software trigger signal
                // reset to false to prevent delivery
                if (trigger_mode_ && !trigger_next_image_)
                {
                    send_image = false;
                }
                if (trigger_mode_ && trigger_next_image_)
                {
                    trigger_next_image_ = false;
                }
            }

            if (stream_thread_ended_)
            {
                break;
            }
        }

        if (send_image)
        {
            next_time = std::chrono::steady_clock::now() + send_interval;

            std::shared_ptr<ImageBuffer> buf = fetch_free_buffer();
            if (buf)
            {
                // SPDLOG_ERROR("next image"); //

                auto dst = buf->get_img_descriptor();

                generator_->step();
                generator_->fill_image(dst);

                tcam_stream_statistics stats = {};
                stats.frame_count = frames_delivered_;
                stats.frames_dropped = frames_dropped_;

                auto end = std::chrono::high_resolution_clock::now();
                stats.capture_time_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_time_).count();

                buf->set_statistics(stats);
                buf->set_valid_data_length(buf->get_image_buffer_size());

                stream_sink_->push_image(buf);
                ++frames_delivered_;
            }
            else
            {
                ++frames_dropped_;
            }
        }
    }
}


std::shared_ptr<tcam::ImageBuffer> tcam::virtcam::VirtcamDevice::fetch_free_buffer()
{
    std::scoped_lock lck { buffer_queue_mutex_ };

    if (buffer_queue_.empty())
    {
        return {};
    }
    auto buf = buffer_queue_.front();
    buffer_queue_.erase(buffer_queue_.begin());

    return buf;
}
