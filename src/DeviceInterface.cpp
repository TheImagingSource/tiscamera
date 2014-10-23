


#include "DeviceInterface.h"
#include "logging.h"

#include "config.h"

#if HAVE_USB
#include "V4l2Device.h"
#endif

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
            case TIS_DEVICE_TYPE_V4L2:
            {
#if HAVE_USB
                return std::make_shared<V4l2Device>(device);
#else
                break;
#endif
            }
            case TIS_DEVICE_TYPE_ARAVIS:
            {
#if HAVE_ARAVIS
                return std::make_shared<AravisDevice>(device);
#else
                break;
#endif
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
    return nullptr;
}
