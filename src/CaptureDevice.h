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

#include "DeviceIndex.h"
#include "DeviceInfo.h"
#include "Properties.h"
#include "VideoFormat.h"
#include "SinkInterface.h"
#include "VideoFormatDescription.h"
#include "standard_properties.h"

#include <string>
#include <vector>
#include <memory>

/**
 * @addtogroup API
 * @{
 */

namespace tcam
{

class CaptureDeviceImpl;

class CaptureDevice
{

public:

    CaptureDevice ();
    CaptureDevice (const DeviceInfo&);

    CaptureDevice (const CaptureDevice&) = delete;

    CaptureDevice operator= (const CaptureDevice&) = delete;

    ~CaptureDevice ();

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


    // property related:

    /**
     * @return vector containing all available properties
     */
    std::vector<Property*> get_available_properties ();


    Property* get_property (TCAM_PROPERTY_ID id);
    Property* get_property_by_name (const std::string& name);


    /**
     *
     */
    template<class TProperty>
    TProperty* find_property (TCAM_PROPERTY_ID id)
    {
        for (auto p : get_available_properties())
        {
            if (p->get_ID() == id)
            {

                if (get_reference_property_type(id) != TProperty::type)
                {
                    // TODO replace with static_assert
                    return nullptr;
                }

                static auto prop_desc = create_empty_property(id);

                return (TProperty*) p;

            }
        }
        return nullptr;
    }


    bool set_property (TCAM_PROPERTY_ID, const int64_t& value);
    bool set_property (TCAM_PROPERTY_ID, const double& value);
    bool set_property (TCAM_PROPERTY_ID, const bool& value);
    bool set_property (TCAM_PROPERTY_ID, const std::string& value);

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

    std::unique_ptr<CaptureDeviceImpl> impl;

}; /* class CaptureDevice */

std::shared_ptr<CaptureDevice> open_device (const std::string& serial);

} /* namespace tcam */

/** @} */

#endif /* TCAM_CAPTUREDEVICE_H */
