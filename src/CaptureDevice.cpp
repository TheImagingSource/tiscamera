


#include "CaptureDevice.h"
#include "device_discovery.h"

#include <cstring>

using namespace tis_imaging;

    
CaptureDevice::CaptureDevice (const struct tis_device_info& _device)
    : device(_device)
{}


CaptureDevice::CaptureDevice ()
{
    device.type = TIS_DEVICE_TYPE_UNKNOWN;

    memset(device.identifier, 0, sizeof(device.identifier));
    memset(device.name, 0, sizeof(device.name));
    memset(device.serial_number, 0, sizeof(device.serial_number));
}


CaptureDevice& CaptureDevice::operator= (const CaptureDevice& other)
{
    this->device = other.device;
    return *this;
}


struct tis_device_info CaptureDevice::getInfo () const
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


enum TIS_DEVICE_TYPE CaptureDevice::getDeviceType () const
{
    return device.type;
}


std::string CaptureDevice::getDeviceTypeAsString () const
{
    switch (device.type)
    {
        case TIS_DEVICE_TYPE_GIGE:
            return "GigE";
        case TIS_DEVICE_TYPE_V4L2:
            return "V4L2";
        case TIS_DEVICE_TYPE_FIREWIRE:
            return "Firewire";
        default:
            return "";
    }
}


std::vector<CaptureDevice> tis_imaging::getAvailableCaptureDevices ()
{
    int count = tis_get_camera_count();

    std::vector<struct tis_device_info> info(count);
    int ret = tis_get_camera_list(info.data(), count);

    auto vec = std::vector<CaptureDevice>();
    
    if (ret < -1)
    {
        return vec;
    }

    for (const auto& i : info)
    {
        vec.push_back(CaptureDevice(i));
    }

    return vec;
}

