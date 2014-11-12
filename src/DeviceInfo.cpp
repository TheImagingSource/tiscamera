


#include "DeviceInfo.h"

#include <cstring>

using namespace tcam;


DeviceInfo::DeviceInfo (const struct tcam_device_info& device_desc)
    : device(device_desc)
{}


DeviceInfo::DeviceInfo ()
{
    device.type = TCAM_DEVICE_TYPE_UNKNOWN;

    memset(device.identifier, 0, sizeof(device.identifier));
    memset(device.name, 0, sizeof(device.name));
    memset(device.serial_number, 0, sizeof(device.serial_number));
}


DeviceInfo& DeviceInfo::operator= (const DeviceInfo& other)
{
    this->device = other.device;
    return *this;
}


struct tcam_device_info DeviceInfo::getInfo () const
{
    return device;
}


std::string DeviceInfo::getName () const
{
    return device.name;
}


std::string DeviceInfo::getSerial () const
{
    return device.serial_number;
}


std::string DeviceInfo::getIdentifier () const
{
    return device.identifier;
}


enum TCAM_DEVICE_TYPE DeviceInfo::getDeviceType () const
{
    return device.type;
}


std::string DeviceInfo::getDeviceTypeAsString () const
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
