
#include "Grabber.h"

#include "tis_logging.h"


using namespace tis_imaging;


Grabber::Grabber ()
{}


Grabber::~Grabber ()
{}


bool Grabber::openDevice (const CaptureDevice& _device)
{

    open_device = _device;

    capture = std::make_shared<Device>(open_device);

    return true;
}

bool Grabber::isDeviceOpen () const
{
    if (capture != nullptr)
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
    capture.reset();

    tis_log(TIS_LOG_INFO, "Closed device %s.", name.c_str());
    
    return true;
}


std::vector<Property> Grabber::getAvailableProperties ()
{
    if (!isDeviceOpen())
    {
        return std::vector<Property>();
    }
    return capture->getProperties();
}


std::vector<VideoFormatDescription> Grabber::getAvailableVideoFormats () const
{
    if (!isDeviceOpen())
    {
        return std::vector<VideoFormatDescription>();
    }
    
    return capture->getAvailableVideoFormats();
}


bool Grabber::setVideoFormat (const VideoFormat& _format)
{
    if (!isDeviceOpen())
    {
        return false;
    }
        
    return this->capture->setVideoFormat(_format);
}


VideoFormat Grabber::getActiveVideoFormat () const
{
    if(!isDeviceOpen())
    {
        return VideoFormat();
    }
        
    return capture->getActiveVideoFormat();
}

