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

#ifndef TCAM_CAPTUREINTERFACE_H
#define TCAM_CAPTUREINTERFACE_H

#include "DeviceInfo.h"
#include "ImageBuffer.h"
#include "BufferPool.h"
#include "Allocator.h"
#include "PropertyInterfaces.h"
#include "SinkInterface.h"
#include "VideoFormat.h"
#include "VideoFormatDescription.h"
#include "compiler_defines.h"

#include <memory>
#include <vector>

VISIBILITY_INTERNAL

namespace tcam
{

class DeviceInterface;

    std::vector<DeviceInfo> get_device_list();

    // open device interface correlating to device
    // returns nullptr and logs error on failure
    std::shared_ptr<DeviceInterface> open_device_interface(const DeviceInfo& device);

class DeviceInterface : public IImageBufferPool
{

public:
    virtual ~DeviceInterface() = default;

    /**
     * @return the DeviceInfo describing the device
     */
    virtual DeviceInfo get_device_description() const = 0;

    virtual std::vector<std::shared_ptr<tcam::property::IPropertyBase>> get_properties() = 0;

    /**
     * @brief Set Format in he actual device
     * @return True on success; False on error or invalid format
     */
    virtual bool set_video_format(const VideoFormat&) = 0;

    /**
     * @return The current VideoFormat that is set in the device
     */
    virtual VideoFormat get_active_video_format() const = 0;

    /**
     * Retrieve all formats the device supports
     * @return vector containing all supported formats; empty on error
     */
    virtual std::vector<VideoFormatDescription> get_available_video_formats() = 0;

    virtual std::shared_ptr<tcam::AllocatorInterface> get_allocator() = 0;

    /**
     * @return true on successfull allocation/registration; else false
     */
    virtual bool initialize_buffers(std::shared_ptr<BufferPool> pool) = 0;

    /**
     * @brief Delete all internal references to used memorybuffers
     */
    virtual bool release_buffers() = 0;

    //virtual void requeue_buffer(const std::shared_ptr<ImageBuffer>&) = 0;

    /**
     * Start image retrieval and wait for new images
     * A SinkInterface has to be given via @set_sink
     * @return true on success; else false
     */
    virtual bool start_stream(const std::shared_ptr<IImageBufferSink>&) = 0;

    /**
     * Stop image retrieval
     * @return true on success; else false
     */
    virtual void stop_stream() = 0;

    bool register_device_lost_callback(tcam_device_lost_callback callback, void* user_data)
    {
        struct callback_container cc = { callback, user_data };

        lost_callbacks.push_back(cc);

        return true;
    }

    void set_drop_incomplete_frames(bool b)
    {
        drop_incomplete_frames_ = b;
    }

    virtual outcome::result<tcam::framerate_info> get_framerate_info(const VideoFormat& fmt);

protected:
    DeviceInfo device;

    void notify_device_lost()
    {
        auto dev = device.get_info();

        for (const auto& cc : lost_callbacks) { cc.callback(&dev, cc.user_data); }
    }

    bool drop_incomplete_frames_ = true;

private:
    struct callback_container
    {
        tcam_device_lost_callback callback;
        void* user_data;
    };

    std::vector<callback_container> lost_callbacks;
}; /* class Camera_Interface */


/**
 * @brief open the device for the given DeviceInfo
 * @param device - device description for which an interface shall be created
 * @return shared_ptr containing the device; nullptr on error
 */
std::shared_ptr<DeviceInterface> open_device_interface(const DeviceInfo& device);

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_CAPTUREINTERFACE_H */
