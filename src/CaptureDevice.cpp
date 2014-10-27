


#include "CaptureDevice.h"
#include "device_discovery.h"

#include <cstring>

using namespace tcam;


CaptureDevice::CaptureDevice (const struct tcam_device_info& device_desc)
    : device(device_desc)
{}


CaptureDevice::CaptureDevice ()
{
    device.type = TCAM_DEVICE_TYPE_UNKNOWN;

    memset(device.identifier, 0, sizeof(device.identifier));
    memset(device.name, 0, sizeof(device.name));
    memset(device.serial_number, 0, sizeof(device.serial_number));
}


CaptureDevice& CaptureDevice::operator= (const CaptureDevice& other)
{
    this->device = other.device;
    return *this;
}


struct tcam_device_info CaptureDevice::getInfo () const
{
    return device;
}


std::string CaptureDevice::getName () const
{
    return device.name;
}


std::string CaptureDevice::getSerial () const
{
    return device.serial_number;
}


std::string CaptureDevice::getIdentifier () const
{
    return device.identifier;
}


enum TCAM_DEVICE_TYPE CaptureDevice::getDeviceType () const
{
    return device.type;
}


std::string CaptureDevice::getDeviceTypeAsString () const
{
    switch (device.type)
    {
        case TCAM_DEVICE_TYPE_V4L2:
            return "V4L2";
        case TCAM_DEVICE_TYPE_ARAVIS:
            return "Aravis";
        case TCAM_DEVICE_TYPE_FIREWIRE:
            return "Firewire";
        default:
            return "";
    }
}
