



#ifndef DEVICE_H_
#define DEVICE_H_

#include "CaptureDevice.h"
#include "CaptureInterface.h"

#include <memory>

namespace tis_imaging
{

struct control_reference;

class Device : public CaptureInterface
{
public:

    Device () = delete;
    
    Device (const CaptureDevice&);
    
    Device (const Device&);
    
    Device& operator= (const Device&) = delete;
    
    ~Device ();

    CaptureDevice getDeviceDescription () const;

    std::vector<Property> getProperties () const;

    std::vector<std::shared_ptr<Property> > create_properties(std::shared_ptr<PropertyImpl>);

    bool isAvailable (const Property&);
    
    bool setProperty (const Property&);
    
    bool getProperty (Property&);
    
    bool setVideoFormat (const VideoFormat&);
    
    VideoFormat getActiveVideoFormat () const;
    
    std::vector<VideoFormatDescription> getAvailableVideoFormats () const;

private:

    std::shared_ptr<CaptureInterface> actual_device;

    std::vector<std::weak_ptr<Property> > device_properties;
    
    std::vector<std::shared_ptr<Property> > user_properties;

    void create_pass_through_property (std::shared_ptr<Property> dev_prop);

    void create_new_user_property (std::shared_ptr<Property>, const control_reference&);

    bool create_user_properties ();
    
}; /* class Device */

} /* namespace tis_imaging */

#endif /* DEVICE_H_ */


