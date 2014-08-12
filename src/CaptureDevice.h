



#ifndef CAPTUREDEVICE_H_
#define CAPTUREDEVICE_H_

#include "base_types.h"

#include <vector>
#include <memory>


namespace tis_imaging
{

class CaptureDevice
{

public:

    
    CaptureDevice (const struct tis_device_info&);

    /**
     * @brief Creates an invalid device  
     */
    CaptureDevice ();

    CaptureDevice (const CaptureDevice&);

    CaptureDevice& operator= (const CaptureDevice&);
    
    ~CaptureDevice ();

    struct tis_device_info getInfo () const;
    
    std::string getName () const;
    
    enum TIS_DEVICE_TYPE getDeviceType () const;

private:

    /// internal device representation
    struct tis_device_info device;
    
}; /* class CaptureDevice */

/// @brief return a list of all identified capture devices
std::shared_ptr<std::vector<CaptureDevice> > getAvailableCaptureDevices ();

} /* namespace tis_imaging */

#endif /* CAPTUREDEVICE_H_ */
