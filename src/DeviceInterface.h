



#ifndef CAPTUREINTERFACE_H_
#define CAPTUREINTERFACE_H_

#include "DeviceInfo.h"
#include "Properties.h"
#include "PropertyImpl.h"

#include "VideoFormat.h"
#include "VideoFormatDescription.h"

#include "MemoryBuffer.h"
#include "SinkInterface.h"

#include <vector>
#include <memory>


namespace tcam
{

class DeviceInterface : public PropertyImpl
{

public:

    virtual ~DeviceInterface () {};

    /**
     * @return the DeviceInfo describing the device
     */
    virtual DeviceInfo getDeviceDescription () const = 0;

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

    /**
     * @return The current VideoFormat that is set in the device
     */
    virtual VideoFormat getActiveVideoFormat () const = 0;

    /**
     * Retrieve all formats the device supports
     * @return vector containing all supported formats; empty on error
     */
    virtual std::vector<VideoFormatDescription> getAvailableVideoFormats () = 0;

    /**
     * Set the ImageSource to which new images shall be delivered
     * This overwrites previously defined Sinks
     * @return true on successful registration; else false
     */
    virtual bool setSink (std::shared_ptr<SinkInterface>) = 0;

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
     * A SinkInterface has to be given via @setSink
     * @return true on success; else false
     */
    virtual bool start_stream () = 0;

    /**
     * Stop image retrieval
     * @return true on success; else false
     */
    virtual bool stop_stream () = 0;

}; /* class Camera_Interface */


/**
 * @brief open the device for the given DeviceInfo
 * @param device - device description for which an interface shall be created
 * @return shared_ptr containing the device; nullptr on error
 */
std::shared_ptr<DeviceInterface> openDeviceInterface (const DeviceInfo& device);

} /* namespace tcam */

#endif /* CAPTUREINTERFACE_H_ */
