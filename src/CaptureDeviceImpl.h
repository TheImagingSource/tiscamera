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

#include "DeviceInfo.h"
#include "DeviceInterface.h"
#include "Properties.h"
#include "PipelineManager.h"
#include "PropertyHandler.h"

#include <string>
#include <vector>
#include <memory>

#include "internal.h"

VISIBILITY_INTERNAL

namespace tcam
{

class CaptureDeviceImpl
{

public:

    explicit CaptureDeviceImpl ();

    explicit CaptureDeviceImpl (const DeviceInfo& device);

    ~CaptureDeviceImpl ();


    /**
     * @brief Load xml configuration and apply it to device
     * @param filename - string containing the filename of the xml description
     * @return true on success; on error Error will be set
     */
    bool load_configuration (const std::string& filename);


    /**
     * @brief Store current device configuration in xml
     * @param filename - string containing the filename under which the xml shall be saved
     * @return true on success; on error Error will be set
     */
    bool save_configuration (const std::string& filename);

    // device related:


    /**
     * Open the described device for interaction
     * @param device - DeviceInfo description of the device that shall be opened
     * @return true on success; on error Error will be set
     */
    bool open_device (const DeviceInfo& device);


    /**
     * Check if device is currently open
     * @return true if a device is open
     */
    bool is_device_open () const;


    /**
     * Return description of current device
     * @return description of the currently open device. empty if no device is open
     */
    DeviceInfo get_device () const;


    bool register_device_lost_callback (tcam_device_lost_callback callback, void* user_data);


    /**
     * Closes the open device. All streams will be stopped.
     * @return true on success; on error Error will be set
     */
    bool close_device ();

    // property related:

    /**
     * @return vector containing all available properties
     */
    std::vector<Property*> get_available_properties ();

    // videoformat related:


    /**
     * @return vector containing all available video format settings
     */
    std::vector<VideoFormatDescription> get_available_video_formats () const;


    /**
     * Description for set_video_format.
     * @param new_format - format the device shall use
     * @return true if device accepted the given VideoFormat
     */
    bool set_video_format (const VideoFormat& new_format);


    /**
     * @return Currently used video format
     */
    VideoFormat get_active_video_format () const;

    // playback related:

    /**
     * @brief Start a new stream
     * @param sink - SinkInterface that shall be called for new images
     * @return true if stream could successfully be initialized
     */
    bool start_stream (std::shared_ptr<SinkInterface> sink);


    /**
     * @brief Stop currently running stream
     * @return true if stream could successfully be stopped
     */
    bool stop_stream ();

private:

    // both need to be shared_ptr and not unique_ptr
    // the property handler is used for callbacks of properties
    // the pipeline is used for callbacks of ImageSource instances
    std::shared_ptr<PipelineManager> pipeline;
    std::shared_ptr<PropertyHandler> property_handler;

    DeviceInfo open_device_info;
    VideoFormat active_format;

    std::shared_ptr<DeviceInterface> device;

}; /* class CaptureDeviceImpl */

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_CAPTUREDEVICEIMPL_H */
