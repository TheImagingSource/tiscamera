


#include "CaptureDevice.h"
#include "tis_camera_index.h"

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


CaptureDevice::CaptureDevice (const CaptureDevice& other)
    : device(other.device)
{}



CaptureDevice& CaptureDevice::operator= (const CaptureDevice& other)
{
    this->device = other.device;
    return *this;
}


CaptureDevice::~CaptureDevice ()
{}


struct tis_device_info CaptureDevice::getInfo () const
{
    return device;
}


std::string CaptureDevice::getName () const
{
    return device.name;
}


enum TIS_DEVICE_TYPE CaptureDevice::getDeviceType () const
{
    return device.type;
}


std::shared_ptr<std::vector<CaptureDevice> > tis_imaging::getAvailableCaptureDevices ()
{
    int count = tis_get_camera_count();

    std::vector<struct tis_device_info> info(count);

    int ret = tis_get_camera_list(info.data(), count);

    if (ret < -1)
    {
        return nullptr;
    }

    auto vec = std::make_shared<std::vector<CaptureDevice> >();


    for (const auto& i : info)
    {
        vec->push_back(CaptureDevice(i));
    }

    return vec;
}

