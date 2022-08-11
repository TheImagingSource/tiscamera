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

#pragma once

#include "../DeviceInterface.h"
#include "../VideoFormatDescription.h"

#include <condition_variable> // std::condition_variable
#include <memory>
#include <chrono>
#include <mutex> // std::mutex, std::unique_lock
#include <thread>


// forward declaration

namespace tcam::generator
{
class IGenerator;
}

namespace tcam::virtcam
{

class VirtcamPropertyBackend;

class VirtcamDevice : public DeviceInterface
{
public:
    explicit VirtcamDevice(const DeviceInfo&);
    VirtcamDevice(const DeviceInfo& info,
                  const std::vector<tcam::VideoFormatDescription>& desc);

    VirtcamDevice() = delete;

    ~VirtcamDevice();

    DeviceInfo get_device_description() const final;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() final
    {
        return properties_;
    }

    bool set_video_format(const VideoFormat&) final;

    VideoFormat get_active_video_format() const final;

    std::vector<VideoFormatDescription> get_available_video_formats() final;

    std::shared_ptr<tcam::AllocatorInterface> get_allocator() final
    {
        return get_default_allocator();
    };

    bool initialize_buffers(std::shared_ptr<BufferPool>) final;

    bool release_buffers() final;

    void requeue_buffer(const std::shared_ptr<ImageBuffer>&) final;

    bool start_stream(const std::shared_ptr<IImageBufferSink>&) final;

    void stop_stream() final;

    void trigger_device_lost();

private:
    std::atomic<bool> m_is_stream_on { false };

    std::thread stream_thread_;
    std::condition_variable stream_thread_cv_;
    std::mutex stream_thread_mutex_;
    bool stream_thread_ended_ = false;

    int frames_dropped_ = 0;
    int frames_delivered_ = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time_;

    VideoFormat active_video_format_;

    std::vector<VideoFormatDescription> available_videoformats_;

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> properties_;

    friend class VirtcamPropertyBackend;

    std::shared_ptr<tcam::virtcam::VirtcamPropertyBackend> property_backend_;


    std::atomic<bool> trigger_mode_ { false };
    std::atomic<bool> trigger_next_image_ { false };

    std::shared_ptr<BufferPool> pool_ = nullptr;

    std::vector<std::shared_ptr<ImageBuffer>> buffer_queue_;
    std::mutex buffer_queue_mutex_;

    std::shared_ptr<IImageBufferSink> stream_sink_;

    std::unique_ptr<tcam::generator::IGenerator> generator_;

    void stream_thread_main();

    std::shared_ptr<ImageBuffer> fetch_free_buffer();

    void generate_properties();
};

} // namespace tcam::virtcam
