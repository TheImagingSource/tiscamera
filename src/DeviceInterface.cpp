


#include "DeviceInterface.h"
#include "logging.h"

#if HAVE_USB
#include "V4l2Device.h"
#endif

#if HAVE_ARAVIS

#include "AravisDevice.h"

#endif

#include <algorithm>
#include <memory>

using namespace tcam;


std::shared_ptr<DeviceInterface> tcam::openDeviceInterface (const DeviceInfo& device)
{

    try
    {
        TCAM_DEVICE_TYPE type = device.get_device_type();

        switch (type)
        {
            case TCAM_DEVICE_TYPE_V4L2:
            {
#if HAVE_USB
                return std::make_shared<V4l2Device>(device);
#else
                break;
#endif
            }
            case TCAM_DEVICE_TYPE_ARAVIS:
            {
#if HAVE_ARAVIS
                return std::make_shared<AravisDevice>(device);
#else
                break;
#endif
            }
            case TCAM_DEVICE_TYPE_FIREWIRE:
            case TCAM_DEVICE_TYPE_UNKNOWN:
            default:
            {
                throw std::runtime_error("Unsupported device");
            }
        }

    }
    catch (...)
    {
        tcam_log(TCAM_LOG_ERROR, "Encountered Error while creating device interface.");
        return nullptr;
    }
    return nullptr;
}
