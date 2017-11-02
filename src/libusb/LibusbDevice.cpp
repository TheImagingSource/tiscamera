
#include "LibusbDevice.h"
#include "UsbHandler.h"

#include "logging.h"

#include <algorithm>

tcam::LibusbDevice::LibusbDevice (std::shared_ptr<tcam::UsbSession> s,
                                  const std::string& serial)
    :session_(s)
{

    device_handle_ = UsbHandler::get_instance().open_device(serial);

    if (!device_handle_)
    {
        tcam_log(TCAM_LOG_ERROR, "Failed to open device.");
    }
}


tcam::LibusbDevice::LibusbDevice (std::shared_ptr<tcam::UsbSession> s, libusb_device* dev)
    :session_(s), device_(dev)
{
    if (dev)
    {
        int ret = libusb_open(dev, &device_handle_);

        if (ret < 0)
        {
            tcam_error("Unable to open device.");
            throw;
        }
    }
    else
    {
        throw;
    }
}


tcam::LibusbDevice::~LibusbDevice ()
{
    for (auto& interface : open_interfaces_)
    {
        close_interface(interface);
    }

    if (device_handle_)
    {
        libusb_close(device_handle_);
    }

    if (device_)
    {
        libusb_unref_device(device_);
    }
}


struct libusb_device_handle* tcam::LibusbDevice::get_handle ()
{
    return device_handle_;
}


bool tcam::LibusbDevice::open_interface (int interface)
{
    if (std::find(open_interfaces_.begin(), open_interfaces_.end(), interface) != open_interfaces_.end())
    {
        tcam_warning("Interface %d is already open.", interface);
        return false;
    }

    int ret = libusb_claim_interface(device_handle_, interface);

    if (ret < 0)
    {
        tcam_error("Could not claim interface %d", interface);
        return false;
    }

    open_interfaces_.push_back(interface);

    return true;
}


bool tcam::LibusbDevice::close_interface (int interface)
{
    int ret = libusb_release_interface(device_handle_, interface);

    if (ret < 0)
    {
        tcam_error("Could not release interface %d", interface);
        return false;
    }

    auto entry = std::find(open_interfaces_.begin(), open_interfaces_.end(), interface);

    if (entry != open_interfaces_.end())
    {
        open_interfaces_.erase(entry);
    }

    return true;
}
