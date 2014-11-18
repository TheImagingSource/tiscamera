
#include "CaptureDevice.h"

#include "Error.h"

#include "logging.h"
#include "utils.h"
#include "serialization.h"

#include "CaptureDeviceImpl.h"

using namespace tcam;


CaptureDevice::CaptureDevice ()
    : impl(new CaptureDeviceImpl())
{}


CaptureDevice::~CaptureDevice ()
{
    if (isDeviceOpen())
        closeDevice();
}


bool CaptureDevice::load_configuration (const std::string& filename)
{
    return impl->load_configuration(filename);
}


bool CaptureDevice::save_configuration (const std::string& filename)
{
    return impl->save_configuration(filename);
}


bool CaptureDevice::openDevice (const DeviceInfo& device_desc)
{
    return impl->openDevice(device_desc);
}


bool CaptureDevice::isDeviceOpen () const
{
    return impl->isDeviceOpen ();
}


DeviceInfo CaptureDevice::getDevice () const
{
    return impl->getDevice();
}


bool CaptureDevice::closeDevice ()
{
    return impl->closeDevice ();
}


std::vector<Property> CaptureDevice::getAvailableProperties () const
{
    return impl->getAvailableProperties();
}


std::vector<VideoFormatDescription> CaptureDevice::getAvailableVideoFormats () const
{
    return impl->getAvailableVideoFormats();
}


bool CaptureDevice::setVideoFormat (const VideoFormat& new_format)
{
    return impl->setVideoFormat(new_format);
}


VideoFormat CaptureDevice::getActiveVideoFormat () const
{
    return impl->getActiveVideoFormat();
}


bool CaptureDevice::startStream (std::shared_ptr<SinkInterface> sink)
{
    return impl->startStream(sink);
}


bool CaptureDevice::stopStream ()
{
    return impl->stopStream();
}
