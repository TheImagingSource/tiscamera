

#ifndef GRABBER_H_
#define GRABBER_H_

#include "CaptureDevice.h"
#include "DeviceInterface.h"
#include "Properties.h"
#include "PipelineManager.h"

#include <string>
#include <vector>
#include <memory>

namespace tcam
{

class Grabber
{

public:

    explicit Grabber ();

    ~Grabber ();


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
     * @param device - CaptureDevice description of the device that shall be opened
     * @return true on success; on error Error will be set
     */
    bool openDevice (const CaptureDevice& device);


    /**
     * Check if device is currently open
     * @return true if a device is open
     */
    bool isDeviceOpen () const;


    /**
     * Return description of current device
     * @return description of the currently open device. empty if no device is open
     */
    CaptureDevice getDevice() const;


    /**
     * Closes the open device. All streams will be stopped.
     * @return true on success; on error Error will be set
     */
    bool closeDevice ();

    // property related:

    std::vector<Property> getAvailableProperties ();

    // videoformat related:

    std::vector<VideoFormatDescription> getAvailableVideoFormats () const;

    bool setVideoFormat (const VideoFormat&);

    VideoFormat getActiveVideoFormat () const;

    // playback related:

    bool startStream (std::shared_ptr<SinkInterface>);

    bool stopStream ();

private:

    std::shared_ptr<PipelineManager> pipeline;

    CaptureDevice open_device;
    VideoFormat active_format;

    // std::shared_ptr<Device> capture;
    std::shared_ptr<DeviceInterface> device;

    std::vector<std::shared_ptr<Property>> device_properties;

    std::vector<std::shared_ptr<Property>> pipeline_properties;

    std::vector<std::shared_ptr<Property>> user_properties;

}; /* class Grabber */

} /* namespace tcam */

#endif /* GRABBER_H_ */
