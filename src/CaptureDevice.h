

#ifndef CAPTUREDEVICE_H
#define CAPTUREDEVICE_H

#include "DeviceInfo.h"
#include "Properties.h"
#include "VideoFormat.h"
#include "SinkInterface.h"
#include "VideoFormatDescription.h"

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
    bool isDeviceOpen () const;

    /**
     * Return description of current device
     * @return description of the currently open device. empty if no device is open
     */
    DeviceInfo getDevice() const;

    // property related:

    /**
     * @return vector containing all available properties
     */
    std::vector<Property> getAvailableProperties () const;

    // videoformat related:


    /**
     * @return vector containing all available video format settings
     */
    std::vector<VideoFormatDescription> getAvailableVideoFormats () const;


    /**
     * Description for setVideoFormat.
     * @param new_format - format the device shall use
     * @return true if device accepted the given VideoFormat
     */
    bool setVideoFormat (const VideoFormat& new_format);


    /**
     * @return Currently used video format
     */
    VideoFormat getActiveVideoFormat () const;

    // playback related:

    /**
     * @brief Start a new stream
     * @param sink - SinkInterface that shall be called for new images
     * @return true if stream could successfully be initialized
     */
    bool startStream (std::shared_ptr<SinkInterface> sink);


    /**
     * @brief Stop currently running stream
     * @return true if stream could successfully be stopped
     */
    bool stopStream ();

private:

    std::shared_ptr<CaptureDeviceImpl> impl;

}; /* class CaptureDevice */

} /* namespace tcam */

/** @} */

#endif /* CAPTUREDEVICE_H */
