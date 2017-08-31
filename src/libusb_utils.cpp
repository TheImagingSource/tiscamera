


#include "libusb_utils.h"

#include "UsbHandler.h"

using namespace tcam;


std::vector<DeviceInfo> tcam::get_libusb_device_list ()
{
    return UsbHandler::get_instance().get_device_list();
}
