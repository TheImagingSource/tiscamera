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

#include "DeviceInterface.h"
#include "FormatHandlerInterface.h"
#include "V4L2PropertyBackend.h"
#include "VideoFormat.h"
#include "VideoFormatDescription.h"

#include <atomic>
#include <condition_variable> // std::condition_variable
#include <linux/videodev2.h>
#include <memory>
#include <mutex> // std::mutex, std::unique_lock
#include <thread>

VISIBILITY_INTERNAL

namespace tcam
{

class V4l2Device : public DeviceInterface
{

    class V4L2FormatHandler : public FormatHandlerInterface
    {
        friend class V4l2Device;

    public:
        V4L2FormatHandler(V4l2Device*);
        std::vector<double> get_framerates(const struct tcam_image_size&,
                                           int pixelformat = 0) final;

    protected:
        V4l2Device* device;
    };

public:
    explicit V4l2Device(const DeviceInfo&);

    V4l2Device() = delete;

    ~V4l2Device();

    DeviceInfo get_device_description() const override;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return m_properties;
    };

    bool set_video_format(const VideoFormat&) override;

    bool validate_video_format(const VideoFormat&) const;

    VideoFormat get_active_video_format() const override;

    std::vector<VideoFormatDescription> get_available_video_formats() override;

    bool set_framerate(double framerate);

    double get_framerate();

    bool set_sink(std::shared_ptr<SinkInterface>) override;

    bool initialize_buffers(std::vector<std::shared_ptr<ImageBuffer>>) override;

    bool release_buffers() override;

    void requeue_buffer(std::shared_ptr<ImageBuffer>) override;

    bool start_stream() override;

    bool stop_stream() override;

    bool is_lost() const;

private:
    std::atomic<bool> m_is_stream_on { false };
    std::atomic<bool> m_is_lost { false };

    std::thread m_work_thread;
    std::thread m_notification_thread;

    int m_fd = -1;

    VideoFormat m_active_video_format;

    std::vector<VideoFormatDescription> m_available_videoformats;
    bool emulate_bayer = false;

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

    std::shared_ptr<V4L2FormatHandler> m_format_handler;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> m_properties;

    std::thread m_monitor_v4l2_thread;
    std::atomic<bool> m_stop_monitor_v4l2_thread { false };
    int udev_monitor_pipe[2] = { 0, 0 };

    void monitor_v4l2_thread_func();


    void notify_device_lost_func();

    void lost_device();

    void update_stream_timeout();

    /**
     * @brief iterate over all v4l2 format descriptions and convert them
     *        into the internal representation
     */
    void index_formats();

    std::vector<double> index_framerates(const struct v4l2_frmsizeenum& frms);

    void determine_active_video_format();

    bool extension_unit_is_loaded();

    std::shared_ptr<tcam::property::IPropertyBase> new_control(struct v4l2_queryctrl* qctrl);
    void index_controls();
    void sort_properties(
        std::map<uint32_t, std::shared_ptr<tcam::property::IPropertyBase>> properties);

    std::shared_ptr<tcam::property::V4L2PropertyBackend> p_property_backend;

    // streaming related

    // on initial startup all buffers are received once, but empty
    // this causes unneccessary error messages
    // filter those messages until we receive one valid image
    bool m_already_received_valid_image = false;

    struct tcam_stream_statistics m_statistics = {};

    struct buffer_info
    {
        std::shared_ptr<ImageBuffer> buffer;
        bool is_queued;
    };

    std::atomic<int> m_stream_timeout_sec { 10 };

    // std::vector<std::shared_ptr<ImageBuffer>> buffers;
    std::vector<buffer_info> m_buffers;

    std::weak_ptr<SinkInterface> m_listener;

    void stream();

    bool get_frame();

    void init_userptr_buffers();

    bool is_trigger_mode_enabled();

    tcam_image_size get_sensor_size() const;
};

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_V4L2DEVICE_H */
