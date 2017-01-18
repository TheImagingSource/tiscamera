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
#include "Properties.h"
#include "PropertyImpl.h"

#include "VideoFormat.h"
#include "VideoFormatDescription.h"

#include "MemoryBuffer.h"
#include "SinkInterface.h"

#include <vector>
#include <memory>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

class DeviceInterface : public PropertyImpl
{

public:

    virtual ~DeviceInterface () {};

    /**
     * @return the DeviceInfo describing the device
     */
    virtual DeviceInfo get_device_description () const = 0;

    /**
     * @brief Returns all device properties
     */
    virtual std::vector<std::shared_ptr<Property>> getProperties () = 0;

    virtual bool set_property (const Property&) = 0;

    virtual bool get_property (Property&) = 0;

    /**
     * @brief Set Format in he actual device
     * @return True on success; False on error or invalid format
     */
    virtual bool set_video_format (const VideoFormat&) = 0;

    /**
     * @return The current VideoFormat that is set in the device
     */
    virtual VideoFormat get_active_video_format () const = 0;

    /**
     * Retrieve all formats the device supports
     * @return vector containing all supported formats; empty on error
     */
    virtual std::vector<VideoFormatDescription> get_available_video_formats () = 0;

    /**
     * Set the ImageSource to which new images shall be delivered
     * This overwrites previously defined Sinks
     * @return true on successful registration; else false
     */
    virtual bool set_sink (std::shared_ptr<SinkInterface>) = 0;

    /**
     * @return true on successfull allocation/registration; else false
     */
    virtual bool initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>>) = 0;

    /**
     * @brief Delete all internal references to used memorybuffers
     */
    virtual bool release_buffers () = 0;

    /**
     * Start image retrieval and wait for new images
     * A SinkInterface has to be given via @set_sink
     * @return true on success; else false
     */
    virtual bool start_stream () = 0;

    /**
     * Stop image retrieval
     * @return true on success; else false
     */
    virtual bool stop_stream () = 0;

    virtual bool register_device_lost_callback (tcam_device_lost_callback callback, void* user_data)
    {
        struct callback_container cc = {callback, user_data};

        lost_callbacks.push_back(cc);

        return true;
    }

protected:

    DeviceInfo device;

    struct callback_container
    {
        tcam_device_lost_callback callback;
        void* user_data;
    };

    std::vector<callback_container> lost_callbacks;

    virtual void notify_device_lost ()
    {
        auto dev = device.get_info();

        for (const auto& cc : lost_callbacks)
        {
            cc.callback(&dev, cc.user_data);
        }
    }

}; /* class Camera_Interface */


/**
 * @brief open the device for the given DeviceInfo
 * @param device - device description for which an interface shall be created
 * @return shared_ptr containing the device; nullptr on error
 */
std::shared_ptr<DeviceInterface> openDeviceInterface (const DeviceInfo& device);

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_CAPTUREINTERFACE_H */
