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

#include <algorithm>
#include <cstring> /* memcpy*/
#include <dutils_img/image_transform_base.h>
#include <sys/mman.h>

tcam::virtcam::VirtcamDevice::VirtcamDevice(const DeviceInfo& info)
{
    device = info;

    tcam_video_format_description desc_list[] = {
        { FOURCC_MONO8, "Mono8?" },
        { FOURCC_RGGB16, "RGGB16?" },
    };
    tcam_resolution_description res_type = {
        TCAM_RESOLUTION_TYPE_FIXED,
        { 640, 480 },
        { 640, 480 },
        1, 1,
        image_scaling {},
    };
    framerate_mapping m = { res_type, { 15., 30. } };

    for (auto&& d : desc_list)
    {
        available_videoformats_.push_back(tcam::VideoFormatDescription(nullptr, d, { m }));
    }
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

    for (unsigned int i = 0; i < b.size(); ++i)
    {
        //tcam::buffer_info info = { b.at(i), false };
        auto buf = b.at(i).lock();
        int res = mprotect(buf->get_image_buffer_ptr(), buf->get_image_buffer_size(), PROT_READ);
        if (res != 0)
        {
            SPDLOG_WARN("MProtect failed with errno={}", errno);
        }
        buffer_queue_.push_back(buf);
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

    stream_sink_ = sink;

    stream_thread_ended_ = false;
    stream_thread_ = std::thread([this] { stream_thread_main(); });

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

void tcam::virtcam::VirtcamDevice::stream_thread_main()
{
    const int64_t timeout_in_us = 1'000'000 / active_video_format_.get_framerate();

    const auto send_interval = std::chrono::microseconds(timeout_in_us);

    std::chrono::steady_clock::time_point next_time = std::chrono::steady_clock::now();

    while (true)
    {
        bool send_image = false;

        {
            std::unique_lock lck { stream_thread_mutex_ };

            if (!stream_thread_ended_)
            {
                auto res = stream_thread_cv_.wait_until(lck, next_time);
                send_image = res == std::cv_status::timeout;
            }

            if (stream_thread_ended_)
                break;
        }

        if (send_image)
        {
            next_time = std::chrono::steady_clock::now() + send_interval;

            std::shared_ptr<ImageBuffer> buf = fetch_free_buffer();
            if (buf)
            {
                // fill buffer
                fill_buffer(*buf);

                tcam_stream_statistics stats = {};
                stats.frame_count = frames_delivered_;
                stats.frames_dropped = frames_dropped_;
                stats.capture_time_ns = 0;

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

static void fill_buffer_MONO8(img::img_descriptor dst, int frames_delivered)
{
    uint8_t clr0 = ((128 + frames_delivered) * 2) % 255;
    uint8_t clr1 = ((148 + frames_delivered) * 2) % 255;

    for (int y = 0; y < dst.dim.cy; y += 2)
    {
        auto line_start0 = img::get_line_start(dst, y + 0);
        auto line_start1 = img::get_line_start(dst, y + 1);

        memset(line_start0, clr0, dst.dim.cx);
        memset(line_start1, clr1, dst.dim.cx);
    }
}

static void fill_buffer_RGGB16(img::img_descriptor dst, int frames_delivered)
{
    uint16_t clr0 = (((128 + frames_delivered) * 2) % 255) << 8;
    uint16_t clr1 = (((148 + frames_delivered) * 2) % 255) << 8;

    for (int y = 0; y < dst.dim.cy; y += 2)
    {
        auto line_start0 = img::get_line_start<uint16_t>(dst, y + 0);
        auto line_start1 = img::get_line_start<uint16_t>(dst, y + 1);

        for (int x = 0; x < dst.dim.cx; x += 2)
        {
            line_start0[x] = clr0;
            line_start1[x] = clr1;
        }
    }
}

void tcam::virtcam::VirtcamDevice::fill_buffer(ImageBuffer& buf)
{
    auto dst = buf.get_img_descriptor();
    switch (dst.fourcc_type())
    {
        case img::fourcc::MONO8:
            fill_buffer_MONO8(dst, frames_delivered_);
            break;
        case img::fourcc::RGGB16:
            fill_buffer_RGGB16(dst, frames_delivered_);
            break;
        default:
            break;
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
