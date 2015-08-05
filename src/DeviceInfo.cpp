


#include "DeviceInfo.h"

#include <cstring>

using namespace tcam;


DeviceInfo::DeviceInfo (const struct tcam_device_info& device_desc)
    : device(device_desc)
{}


DeviceInfo::DeviceInfo ()
{
    device.type = TCAM_DEVICE_TYPE_UNKNOWN;

    memcpy(device.identifier, "\0", sizeof(device.identifier));
    memcpy(device.name, "\0", sizeof(device.name));
    memcpy(device.serial_number, "\0", sizeof(device.serial_number));
}


DeviceInfo& DeviceInfo::operator= (const DeviceInfo& other)
{
    this->device = other.device;
    return *this;
}


struct tcam_device_info DeviceInfo::get_info () const
{
    return device;
}


std::string DeviceInfo::get_name () const
{
    return device.name;
}


std::string DeviceInfo::get_serial () const
{
    return device.serial_number;
}


std::string DeviceInfo::get_identifier () const
{
    return device.identifier;
}


enum TCAM_DEVICE_TYPE DeviceInfo::get_device_type () const
{
    return device.type;
}


std::string DeviceInfo::get_device_type_as_string () const
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
