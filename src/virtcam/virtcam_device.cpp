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

using namespace img::by_transform;


tcam::virtcam::VirtcamDevice::VirtcamDevice(const DeviceInfo& info)
{
    device = info;

    tcam_video_format_description desc_list[] = {
        { FOURCC_MONO8, "Mono8?" },
        { FOURCC_RGGB12_MIPI_PACKED, "RGGB12p?" },
        { FOURCC_GRBG12_MIPI_PACKED, "GRBG12p?" },
        { FOURCC_RGGB8, "RGGB8?" },
        { FOURCC_RGGB16, "RGGB16?" },
    };
    tcam_resolution_description res_type = {
        TCAM_RESOLUTION_TYPE_RANGE,
        { 640, 480 },
        { 1920, 1080 },
        16, 4,
        image_scaling {},
    };
    framerate_mapping m = { res_type, { 15., 30., 60.} };

    for (auto&& d : desc_list)
    {
        available_videoformats_.push_back(tcam::VideoFormatDescription(nullptr, d, { m }));
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


struct line_values
{
    uint16_t v0, v1;
};


struct bayer16
{
    uint16_t v0, v1;
};

using namespace img::pixel_type;


template<typename T> T generate_struct(line_values vals) = delete;


template<> bayer16 generate_struct<bayer16>(line_values vals)
{
    return bayer16 { vals.v0, vals.v1 };
}


template<> RAW12_MIPI_PACKED generate_struct<RAW12_MIPI_PACKED>(line_values vals)
{
    return RAW12_MIPI_PACKED { vals.v0 >> 8,
                               vals.v1 >> 8,
                               (vals.v0 >> 4 & 0x0F) | (vals.v1 >> 4 & 0x0F) << 4 };
}


template<class TStructType, int struct_step_size, int x_step_size>
void fill_image(img::img_descriptor dst, line_values even, line_values odd)
{
//    SPDLOG_ERROR("Filling {}:{} - {}:{}",even.v0, even.v1, odd.v0, odd.v1);

    for (int y = 0; y < dst.dim.cy; y += 2)
    {
        auto line_start0 = img::get_line_start<TStructType>(dst, y + 0);
        auto line_start1 = img::get_line_start<TStructType>(dst, y + 1);

        int struct_index = 0;
        for (int x = 0; x < dst.dim.cx; x += x_step_size)
        {
            line_start0[struct_index] = generate_struct<TStructType>(even);
            line_start1[struct_index] = generate_struct<TStructType>(odd);

            struct_index += struct_step_size;
        }
    }
}

struct DefStepSize
{
    int pix_max;
    uint16_t step_size;
};

static const DefStepSize default_step_size[] = {

    { 255, 1 },
    { 4095, 63 },
    { 65535, 83 },
};

uint16_t get_default_step_size(uint16_t max_pix)
{
    for (const auto& entry : default_step_size)
    {
        if (entry.pix_max == max_pix)
        {
            return entry.step_size;
        }
    }

    return 1;
}


struct PixCalc
{
    uint16_t CLR_MAX = 4095;
    uint16_t speed_ = 63;
    //calc_speed(CLR_MAX);

    uint16_t r_ = 0;
    uint16_t g_ = 0;
    uint16_t b_ = 0;

    uint16_t* rising_ = &r_;
    uint16_t* descending_ = nullptr;

    PixCalc()
    {
        SPDLOG_ERROR("def");

        CLR_MAX = INT16_MAX;
        speed_ = 255;
    }
    PixCalc(uint16_t max)
    {
        SPDLOG_ERROR("const");
        CLR_MAX = max;
        speed_ = 63;

        r_ = 0;
        g_ = 0;
        b_ = 0;

        rising_ = &r_;
        descending_ = nullptr;
    }

    PixCalc(img::fourcc fcc)
    {
        CLR_MAX = pow(2, img::get_bits_per_pixel(fcc)) - 1;

        speed_ = get_default_step_size(CLR_MAX);
    }

    void step()
    {
        if (rising_)
        {
            *rising_ += speed_;
            SPDLOG_ERROR("ris {} -> {}", speed_, *rising_);
        }

        if (descending_)
        {
            SPDLOG_ERROR("desc");

            *descending_ -= speed_;
        }

        if ((r_ >= CLR_MAX && g_ == 0 && b_ == 0))
        {
            rising_ = &g_;
            descending_ = nullptr;
        }
        else if (r_ >= CLR_MAX && g_ >= CLR_MAX && b_ == 0)
        {
            rising_ = nullptr;
            descending_ = &r_;
        }
        else if (g_ >= CLR_MAX && r_ == 0 && b_ == 0)
        {
            rising_ = &b_;
            descending_ = nullptr;
        }
        else if (r_ == 0 && g_ >= CLR_MAX && b_ >= CLR_MAX)
        {
            rising_ = nullptr;
            descending_ = &g_;
        }
        else if (r_ == 0 && b_ >= CLR_MAX && g_ == 0)
        {
            rising_ = &r_;
            descending_ = nullptr;
        }
        else if (g_ == 0 && b_ >= CLR_MAX && r_ >= CLR_MAX)
        {
            rising_ = nullptr;
            descending_ = &b_;
        }
        SPDLOG_ERROR("step {}:{}:{}", r_,g_,b_);
    }

    // void fill(T& pixel, by_pattern pattern)
    // {
    //     switch (pattern)
    //     {
    //         case by_pattern::BG:
    //         {
    //             pixel = b_;
    //             break;
    //         }
    //         case by_pattern::GB:
    //         case by_pattern::GR:
    //         {
    //             pixel = g_;
    //             break;
    //         }
    //         case by_pattern::RG:
    //         {
    //             pixel = r_;
    //             break;
    //         }
    //     }
    // }

    line_values generate_even()
    {
        return {r_, g_};
    }

    line_values generate_odd()
    {

        return {g_, b_};
    }
};


void f(img::img_descriptor dst)
{
    // if (dst.fourcc_type() == img::fourcc::RGGB16)
    // {
    //     auto even_values = generate_even(dst.fourcc_type());
    //     auto odd_values = generate_odd(dst.fourcc_type());


    //     fill_image<bayer16, 1, 2>(dst, even_values, odd_values);
    // }
    // else
    if (dst.fourcc_type() == img::fourcc::RGGB12_MIPI_PACKED)
    {
        static bool is_init;
        static PixCalc p;
        if (!is_init)
        {
            //    p = PixCalc(4095) ;
            is_init = true;
        }
        p.step();

        auto even = p.generate_even();
        auto odd = p.generate_odd();
        fill_image<RAW12_MIPI_PACKED, 1, 2>(dst, even, odd);
    }
    else if (dst.fourcc_type() == img::fourcc::GRBG12_MIPI_PACKED)
    {
        static bool is_init;
        static PixCalc p;
        if (!is_init)
        {
            //    p = PixCalc(4095) ;
            is_init = true;
        }
        p.step();

        auto even = p.generate_even();
        auto odd = p.generate_odd();
        fill_image<RAW12_MIPI_PACKED, 1, 2>(dst, even, odd);
    }
}

template<typename T>
constexpr T calc_speed(T max)
{
    if (std::is_same<T, uint8_t>::value)
    {
        return 1;
    }
    else if (std::is_same<T, uint16_t>::value)
    {
        return 85;
    }
    else
    {
        return 63;
        T tmp = (max >> (sizeof(T) * sizeof(T)));

        tmp -= max % tmp;

        return tmp;
    }
}

template<typename T>
struct PixelCalc
{
    static const T CLR_MAX = std::numeric_limits<T>::max();
    T speed_ = calc_speed(CLR_MAX);

    T r_ = 0;
    T g_ = 0;
    T b_ = 0;

    T* rising_ = &r_;
    T* descending_ = nullptr;

    void step()
    {
        if (rising_)
        {
            *rising_ += speed_;
        }

        if (descending_)
        {
            *descending_ -= speed_;
        }

        if ((r_ == CLR_MAX && g_ == 0 && b_ == 0))
        {
            rising_ = &g_;
            descending_ = nullptr;
        }
        else if (r_ == CLR_MAX && g_ == CLR_MAX && b_ == 0)
        {
            rising_ = nullptr;
            descending_ = &r_;
        }
        else if (g_ == CLR_MAX && r_ == 0 && b_ == 0)
        {
            rising_ = &b_;
            descending_ = nullptr;
        }
        else if (r_ == 0 && g_ == CLR_MAX && b_ == CLR_MAX)
        {
            rising_ = nullptr;
            descending_ = &g_;
        }
        else if (r_ == 0 && b_ == CLR_MAX && g_ == 0)
        {
            rising_ = &r_;
            descending_ = nullptr;
        }
        else if (g_ == 0 && b_ == CLR_MAX && r_ == CLR_MAX)
        {
            rising_ = nullptr;
            descending_ = &b_;
        }
    }

    void fill (T& pixel, by_pattern pattern)
    {
        switch (pattern)
        {
            case by_pattern::BG:
            {
                pixel = b_;
                break;
            }
            case by_pattern::GB:
            case by_pattern::GR:
            {
                pixel = g_;
                break;
            }
            case by_pattern::RG:
            {
                pixel = r_;
                break;
            }
        }
    }
};


template<enum by_pattern T>
static void fill_buffer_bayer8(img::img_descriptor dst)
{
    static PixelCalc<uint8_t> pixel;

    pixel.step();

    for (int y = 0; y < dst.dim.cy; y += 2)
    {
        auto line_start0 = (uint8_t*)img::get_line_start<uint8_t>(dst, y + 0);
        auto line_start1 = (uint8_t*)img::get_line_start<uint8_t>(dst, y + 1);

        for (int x = 0; x < dst.dim.cx; x += 2)
        {
            pixel.fill(line_start0[x], T);
            pixel.fill(line_start0[x+1], by_pattern_alg::next_pixel(T));
            auto n = by_pattern_alg::next_line(T);

            pixel.fill(line_start1[x], n);
            pixel.fill(line_start1[x+1], by_pattern_alg::next_pixel(n));
        }
    }

}

template<enum by_pattern T>
static void fill_buffer_bayer16(img::img_descriptor dst)
{
    static PixelCalc<uint16_t> pixel;

    pixel.step();

    for (int y = 0; y < dst.dim.cy; y += 2)
    {
        auto line_start0 = img::get_line_start<uint16_t>(dst, y + 0);
        auto line_start1 = img::get_line_start<uint16_t>(dst, y + 1);

        for (int x = 0; x < dst.dim.cx; x += 2)
        {

            pixel.fill(line_start0[x], T);
            pixel.fill(line_start0[x + 1], by_pattern_alg::next_pixel(T));
            auto n = by_pattern_alg::next_line(T);

            pixel.fill(line_start1[x], n);
            pixel.fill(line_start1[x + 1], by_pattern_alg::next_pixel(n));
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
        case img::fourcc::RGGB8:
            //fill_buffer_bayer8<convert_bayer_fcc_to_bayer8_fcc(dst.fourcc_type())>(dst);
//            fill_buffer_RGGB8(dst, frames_delivered_);
            fill_buffer_bayer8<by_pattern::RG>(dst);
            break;
        case img::fourcc::GRBG8:
//            fill_buffer_RGGB8(dst, frames_delivered_);
            fill_buffer_bayer8<by_pattern::GR>(dst);
            break;
        case img::fourcc::RGGB16:
            fill_buffer_bayer16<by_pattern::RG>(dst);
            break;
        case img::fourcc::GRBG16:
            fill_buffer_bayer16<by_pattern::GR>(dst);
            break;
        case img::fourcc::GBRG16:
            fill_buffer_bayer16<by_pattern::GB>(dst);
            break;
        case img::fourcc::BGGR16:
            fill_buffer_bayer16<by_pattern::BG>(dst);
            break;
        case img::fourcc::RGGB12_MIPI_PACKED:
        case img::fourcc::GRBG12_MIPI_PACKED:
            f(dst);
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
