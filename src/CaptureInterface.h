



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

class CaptureInterface :  public PropertyImpl
{

public:

    virtual ~CaptureInterface () {};

    virtual CaptureDevice getDeviceDescription () const = 0;

    virtual std::vector<Property> getProperties () const = 0;

    virtual std::vector<std::shared_ptr<Property>> create_properties(std::shared_ptr<PropertyImpl>) = 0;

    virtual bool setProperty (const Property&) = 0;

    virtual bool getProperty (Property&) = 0;

    virtual bool setVideoFormat (const VideoFormat&) = 0;
     
    virtual VideoFormat getActiveVideoFormat () const = 0;
    
    virtual std::vector<VideoFormatDescription> getAvailableVideoFormats () const = 0;

}; /* class Camera_Interface*/

std::shared_ptr<CaptureInterface> openCaptureInterface (const CaptureDevice& device);

} /* namespace tis_imaging */

#endif /* CAPTUREINTERFACE_H_ */
