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

#ifndef TCAM_CAPTUREDEVICE_H
#define TCAM_CAPTUREDEVICE_H

#include "DeviceInfo.h"
#include "ImageSink.h"
#include "PropertyInterfaces.h"
#include "SinkInterface.h"
#include "VideoFormat.h"
#include "VideoFormatDescription.h"
#include "compiler_defines.h"

#include <memory>
#include <string>
#include <vector>

VISIBILITY_DEFAULT

namespace tcam
{

class AllocatorInterface;
class BufferPool;
class CaptureDeviceImpl;

class CaptureDevice
{

public:
    CaptureDevice() = delete;
    explicit CaptureDevice(const DeviceInfo&);

    CaptureDevice(const CaptureDevice&) = delete;

    CaptureDevice operator=(const CaptureDevice&) = delete;

    ~CaptureDevice();

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
    std::shared_ptr<tcam::property::IPropertyBase> get_property(const std::string& name);


    // videoformat related:


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


    std::shared_ptr<tcam::AllocatorInterface> get_allocator();

    // playback related:

    bool configure_stream(const VideoFormat& format,
                          std::shared_ptr<ImageSink>& sink,
                          std::shared_ptr<BufferPool> pool);

    bool free_stream();

    /**
     * @brief Start a new stream
     * @return true if stream could successfully be initialized
     */
    bool start_stream();


    /**
     * @brief Stop currently running stream
     * @return true if stream could successfully be stopped
     */
    bool stop_stream();

    void set_drop_incomplete_frames(bool b);

    outcome::result<tcam::framerate_info> get_framerate_info(const VideoFormat& fmt);

private:
    std::shared_ptr<CaptureDeviceImpl> impl;

}; /* class CaptureDevice */

std::shared_ptr<CaptureDevice> open_device(const std::string& serial,
                                           TCAM_DEVICE_TYPE type = TCAM_DEVICE_TYPE_UNKNOWN);

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_CAPTUREDEVICE_H */
