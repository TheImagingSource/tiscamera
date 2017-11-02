

#ifndef TCAM_LIBUSBDEVICE_H
#define TCAM_LIBUSBDEVICE_H

#include <memory>
#include <vector>

#include "UsbSession.h"

namespace tcam
{

class LibusbDevice
{
public:

    LibusbDevice (std::shared_ptr<tcam::UsbSession>, const std::string& serial);
    LibusbDevice (std::shared_ptr<tcam::UsbSession>, libusb_device* dev);

    ~LibusbDevice ();

    struct libusb_device_handle* get_handle ();

    bool open_interface (int interface);
    bool close_interface (int interface);

private:

    std::shared_ptr<tcam::UsbSession> session_;
    libusb_device* device_;
    libusb_device_handle* device_handle_;

    std::vector<int> open_interfaces_;

}; // class LibusbDevice

} // namespace tcam

#endif /* TCAM_LIBUSBDEVICE_H */
