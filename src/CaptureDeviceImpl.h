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

#ifndef TCAM_CAPTUREDEVICEIMPL_H
#define TCAM_CAPTUREDEVICEIMPL_H

#include "DeviceIndex.h"
#include "DeviceInfo.h"
#include "DeviceInterface.h"
#include "ImageSink.h"
#include "VideoFormat.h"
#include "PropertyFilter.h"
#include "BufferPool.h"

#include <memory>
#include <string>
#include <vector>

VISIBILITY_INTERNAL

namespace tcam
{

class PipelineManager;
class DeviceInterface;

class CaptureDeviceImpl :
    public IImageBufferSink,
    public std::enable_shared_from_this<CaptureDeviceImpl>
{

public:
    CaptureDeviceImpl() = delete;

    explicit CaptureDeviceImpl(const DeviceInfo& device) noexcept(false);

    ~CaptureDeviceImpl();

    // device related:

    /**
     * Check if device is currently open
     * @return true if a device is open
     */
    bool is_device_open() const;


    /**
     * Return description of current device
     * @return description of the currently open device. empty if no device is open
     */
    DeviceInfo get_device() const;


    bool register_device_lost_callback(tcam_device_lost_callback callback, void* user_data);

    // property related:

    std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties();

    /**
     * @return vector containing all available video format settings
     */
    std::vector<VideoFormatDescription> get_available_video_formats() const;

    /**
     * Description for set_video_format.
     * @param new_format - format the device shall use
     * @return true if device accepted the given VideoFormat
     */
    bool set_video_format(const VideoFormat& new_format);


    /**
     * @return Currently used video format
     */
    VideoFormat get_active_video_format() const;

    // playback related:

    /**
     * @brief Start a new stream
     * @param format - VideoFormat that shall be used
     * @param sink - SinkInterface that shall be called for new images
     * @param pool - BufferPool that shall be used
     * @return true if stream could successfully be configured
     */
    bool configure_stream(const VideoFormat& format,
                          std::shared_ptr<ImageSink>& sink,
                          std::shared_ptr<BufferPool> pool = nullptr);

    /**
     * @brief explicitly free all stream resources
     * @return true if all resources could be freed
     */
    bool free_stream();

    /**
     * @brief Start a new stream
     * @return true if stream could successfully be initialized
     */
    bool start_stream();

    /**
     * @brief Stop currently running stream
     */
    void stop_stream();

    void set_drop_incomplete_frames(bool b);

    outcome::result<tcam::framerate_info> get_framerate_info(const VideoFormat& fmt);

    std::shared_ptr<tcam::AllocatorInterface> get_allocator();

private:
    void push_image(const std::shared_ptr<ImageBuffer>& buffer) final;

    static void deviceindex_lost_cb(const DeviceInfo&, void* user_data);

    struct device_lost_cb_data
    {
        tcam_device_lost_callback callback = nullptr;
        void* user_data = nullptr;
    };

    device_lost_cb_data device_lost_callback_data_ = {};

    std::shared_ptr<DeviceInterface> device_;

    tcam::DeviceIndex index_;

    std::vector<VideoFormatDescription> available_output_formats_;

    std::shared_ptr<ImageSink> sink_;
    std::shared_ptr<BufferPool> pool_ = nullptr;

    bool apply_software_properties_ = true;
    tcam::stream::filter::SoftwarePropertyWrapper property_filter_;

}; /* class CaptureDeviceImpl */

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_CAPTUREDEVICEIMPL_H */
