



#ifndef CAPTUREINTERFACE_H_
#define CAPTUREINTERFACE_H_

#include "CaptureDevice.h"
#include "Properties.h"
#include "PropertyImpl.h"

#include "VideoFormat.h"
#include <VideoFormatDescription.h>

#include "MemoryBuffer.h"
#include "SinkInterface.h"

#include <vector>
#include <memory>


namespace tis_imaging
{

class DeviceInterface : public PropertyImpl
//, std::enable_shared_from_this<DeviceInterface>
{

public:

    virtual ~DeviceInterface () {};

    virtual CaptureDevice getDeviceDescription () const = 0;

    /**
     * @brief Returns all device properties
     */
    virtual std::vector<std::shared_ptr<Property>> getProperties () = 0;

    virtual bool setProperty (const Property&) = 0;

    virtual bool getProperty (Property&) = 0;

    /**
     * @brief Set Format in he actual device
     * @return True on success; False on error or invalid format
     */
    virtual bool setVideoFormat (const VideoFormat&) = 0;
     
    virtual VideoFormat getActiveVideoFormat () const = 0;
    
    virtual std::vector<VideoFormatDescription> getAvailableVideoFormats () = 0;


    virtual bool setSink (std::shared_ptr<SinkInterface>) = 0;

    virtual bool initialize_buffers (std::vector<std::shared_ptr<MemoryBuffer>>) = 0;

    /**
     * @brief Delete all internal references to used memorybuffers
     */
    virtual bool release_buffers () = 0;

    virtual bool start_stream () = 0;

    virtual bool stop_stream () = 0;

}; /* class Camera_Interface*/


/**
 * @brief
 * @param device - device description for which an interface shall be created
 * @return shared_ptr containing the device; nullptr on error
 */
std::shared_ptr<DeviceInterface> openDeviceInterface (const CaptureDevice& device);

} /* namespace tis_imaging */

#endif /* CAPTUREINTERFACE_H_ */
