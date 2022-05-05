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

#ifndef TCAM_V4L2DEVICE_H
#define TCAM_V4L2DEVICE_H

#include "../DeviceInterface.h"
#include "../VideoFormat.h"
#include "../VideoFormatDescription.h"
#include "../BufferPool.h"
#include "V4L2PropertyBackend.h"
#include "V4L2Allocator.h"

#include <atomic>
#include <condition_variable> // std::condition_variable
#include <linux/videodev2.h>
#include <memory>
#include <mutex> // std::mutex, std::unique_lock
#include <thread>

VISIBILITY_INTERNAL

namespace tcam
{
namespace v4l2
{
class prop_impl_offset_auto_center;
}


class V4l2Device : public DeviceInterface
{
public:
    explicit V4l2Device(const DeviceInfo&);

    V4l2Device() = delete;

    ~V4l2Device();

    DeviceInfo get_device_description() const override;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return m_properties;
    }

    bool set_video_format(const VideoFormat&) override;

    VideoFormat get_active_video_format() const override;

    std::vector<VideoFormatDescription> get_available_video_formats() override;

    bool set_framerate(double framerate);

    double get_framerate();

    std::shared_ptr<tcam::AllocatorInterface> get_allocator() override
    {
        return allocator_;
    };

    bool initialize_buffers(std::shared_ptr<BufferPool> pool) override;

    bool release_buffers() override;

    void requeue_buffer(const std::shared_ptr<ImageBuffer>&) final;

    bool start_stream(const std::shared_ptr<IImageBufferSink>&) final;

    void stop_stream() final;

private:
    std::atomic<bool> m_is_stream_on { false };

    std::thread m_work_thread;

    int m_fd = -1;

    VideoFormat m_active_video_format;

    std::vector<VideoFormatDescription> m_available_videoformats;
    bool m_emulate_bayer = false;

    // v4l2 uses fractions
    // to assure correct handling we store the values received by v4l2
    // to reuse them when necessary
    struct framerate_conv
    {
        double fps;
        unsigned int numerator;
        unsigned int denominator;
    };

    std::vector<framerate_conv> framerate_conversions;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_properties;
    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_internal_properties;

    std::shared_ptr<tcam::AllocatorInterface> allocator_ = nullptr;

    std::thread m_monitor_v4l2_thread;
    std::atomic<bool> m_stop_monitor_v4l2_thread { false };

    void monitor_v4l2_thread_func();

    void notify_device_lost_func();

    void lost_device();

    void update_stream_timeout();


    struct override_mapping
    {
        int override_value;
        int scales_index;
    };

    struct device_scaling
    {
        std::vector<std::shared_ptr<tcam::property::IPropertyBase>> properties;

        std::vector<image_scaling> scales;

        std::vector<override_mapping> override_index;
        ImageScalingType scale_type = ImageScalingType::Unknown;
    };

    device_scaling m_scale;

    void determine_scaling();
    void generate_scales();
    bool set_scaling(const image_scaling& scale);
    image_scaling get_current_scaling();

    void    update_properties( const VideoFormat& current_fmt );

    /**
     * @brief iterate over all v4l2 format descriptions and convert them
     *        into the internal representation
     */
    void index_formats();

    std::vector<double> index_framerates(const struct v4l2_frmsizeenum& frms);

    void determine_active_video_format();

    bool load_extension_unit();
    bool extension_unit_is_loaded();

    void generate_properties( const std::vector<v4l2_queryctrl>& qctrl_list );
    void create_properties();
    void create_videoformat_dependent_properties();
    void update_dependency_information();

    std::shared_ptr<tcam::v4l2::V4L2PropertyBackend> p_property_backend;

    // streaming related

    // on initial startup all buffers are received once, but empty
    // this causes unnecessary error messages
    // filter those messages until we receive one valid image
    bool m_already_received_valid_image = false;

    tcam_stream_statistics m_statistics = {};

    std::shared_ptr<BufferPool> pool_;

    struct buffer_info
    {
        std::weak_ptr<ImageBuffer> buffer;
        bool is_queued = false;
    };

    std::atomic<int> m_stream_timeout_sec { 10 };

    std::vector<buffer_info> m_buffers;

    std::weak_ptr<IImageBufferSink> m_listener;

    std::shared_ptr<tcam::v4l2::prop_impl_offset_auto_center>   software_auto_center_;

    void stream();

    bool get_frame();

    void init_userptr_buffers();
    void init_mmap_buffers();
    void init_dma_buffers();

    bool queue_dma(int i, std::shared_ptr<ImageBuffer>);
    bool queue_mmap(int i, std::shared_ptr<ImageBuffer>);
    bool queue_userptr(int i, std::shared_ptr<ImageBuffer>);

    bool is_trigger_mode_enabled();

    tcam_image_size get_sensor_size() const;
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_V4L2DEVICE_H */
