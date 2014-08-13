



#ifndef CAPTUREINTERFACE_H_
#define CAPTUREINTERFACE_H_

#include "CaptureDevice.h"
#include "Properties.h"
#include "PropertyImpl.h"

#include "VideoFormat.h"
#include <VideoFormatDescription.h>

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

    virtual std::vector<std::shared_ptr<Property>> getProperties () = 0;

    virtual bool setProperty (const Property&) = 0;

    virtual bool getProperty (Property&) = 0;

    virtual bool setVideoFormat (const VideoFormat&) = 0;
     
    virtual VideoFormat getActiveVideoFormat () const = 0;
    
    virtual std::vector<VideoFormatDescription> getAvailableVideoFormats () const = 0;

}; /* class Camera_Interface*/

std::shared_ptr<DeviceInterface> openDeviceInterface (const CaptureDevice& device);

} /* namespace tis_imaging */

#endif /* CAPTUREINTERFACE_H_ */
