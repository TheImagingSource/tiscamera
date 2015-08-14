

#ifndef TCAM_CAPTUREDEVICE_H
#define TCAM_CAPTUREDEVICE_H

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

    // property related:

    /**
     * @return vector containing all available properties
     */
    std::vector<Property*> get_available_properties ();


    /**
     *
     */
    template<typename TProperty>
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

    std::shared_ptr<CaptureDeviceImpl> impl;

}; /* class CaptureDevice */

} /* namespace tcam */

/** @} */

#endif /* TCAM_CAPTUREDEVICE_H */
