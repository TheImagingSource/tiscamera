


#include "CaptureInterface.h"

#include "UsbCapture.h"
#include "GigECapture.h"
#include "tis_logging.h"

#include <algorithm>
#include <memory>

using namespace tis_imaging;


std::shared_ptr<CaptureInterface> tis_imaging::openCaptureInterface (const CaptureDevice& device)
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
            {
                return std::make_shared<GigECapture>(device);
            }
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


