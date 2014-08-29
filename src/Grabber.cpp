
#include "Grabber.h"

#include "tis_logging.h"


using namespace tis_imaging;


Grabber::Grabber ()
{}


Grabber::~Grabber ()
{
    if (isDeviceOpen())
        closeDevice();
}


bool Grabber::openDevice (const CaptureDevice& _device)
{

    open_device = _device;

    device = openDeviceInterface(open_device);

    if (device == nullptr)
    {
        return false;
    }
    device_properties = device->getProperties();
    
    return true;
}


bool Grabber::isDeviceOpen () const
{
    if (device != nullptr)
    {
        return true;
    }

    return false;      
}


CaptureDevice Grabber::getDevice () const
{
    return this->open_device;
}


bool Grabber::closeDevice ()
{
    std::string name = open_device.getName();

    open_device = CaptureDevice ();
    device.reset();
    device_properties.clear();

    tis_log(TIS_LOG_INFO, "Closed device %s.", name.c_str());
    
    return true;
}


std::vector<Property> Grabber::getAvailableProperties ()
{
    if (!isDeviceOpen())
    {
        return std::vector<Property>();
    }

    std::vector<Property> props;

    for ( const auto& p : device_properties )
    {
        props.push_back(*p);
    }
    
    return props;
}


std::vector<VideoFormatDescription> Grabber::getAvailableVideoFormats () const
{
    if (!isDeviceOpen())
    {
        return std::vector<VideoFormatDescription>();
    }
    
    return device->getAvailableVideoFormats();
}


bool Grabber::setVideoFormat (const VideoFormat& _format)
{
    if (!isDeviceOpen())
    {
        return false;
    }
        
    return this->device->setVideoFormat(_format);
}


VideoFormat Grabber::getActiveVideoFormat () const
{
    if(!isDeviceOpen())
    {
        return VideoFormat();
    }
        
    return device->getActiveVideoFormat();
}

