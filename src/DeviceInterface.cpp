


#include "DeviceInterface.h"

#include "UsbCapture.h"
#include "tis_logging.h"

#include "config.h"

#if HAVE_ARAVIS

#include "AravisDevice.h"

#endif

#include <algorithm>
#include <memory>

using namespace tis_imaging;


std::shared_ptr<DeviceInterface> tis_imaging::openDeviceInterface (const CaptureDevice& device)
{

    try
    {
        TIS_DEVICE_TYPE type = device.getDeviceType();

        switch (type)
        {
            case TIS_DEVICE_TYPE_USB:
            {
                return std::make_shared<UsbCapture>(device);
            }
            case TIS_DEVICE_TYPE_GIGE:
#if HAVE_ARAVIS
            {
                return std::make_shared<AravisDevice>(device);
            }
#endif
            case TIS_DEVICE_TYPE_FIREWIRE:
            case TIS_DEVICE_TYPE_UNKNOWN:
            default:
            {
                throw std::runtime_error("Unsupported device");
            }
        }

    }
    catch (...)
    {
        tis_log(TIS_LOG_ERROR, "Encountered Error while creating device interface.");
        return nullptr;
    }
    
}


