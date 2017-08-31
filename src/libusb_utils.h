
#ifndef TCAM_LIBUSB_UTILS_H
#define TCAM_LIBUSB_UTILS_H

#include "DeviceInfo.h"

#include <vector>

namespace tcam
{

std::vector<DeviceInfo> get_libusb_device_list ();

} // namespace tcam

#endif /* TCAM_LIBUSB_UTILS_H */
