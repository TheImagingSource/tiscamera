
#include "Grabber.h"

#include "tis_logging.h"


using namespace tis_imaging;


Grabber::Grabber ()
    : device(nullptr), pipeline(std::make_shared<PipelineManager>())
{}


Grabber::~Grabber ()
{
    if (isDeviceOpen())
        closeDevice();
}


bool Grabber::openDevice (const CaptureDevice& _device)
{
    if (pipeline->getPipelineStatus() == PIPELINE_PLAYING ||
        pipeline->getPipelineStatus() == PIPELINE_PAUSED)
    {
        return false;
    }

    if (isDeviceOpen())
    {
        bool ret = closeDevice();
        if (ret == false)
        {
            tis_log(TIS_LOG_ERROR, "Unable to close previous device.");
            return false;
        }
    }

    open_device = _device;

    device = openDeviceInterface(open_device);

    if (device == nullptr)
    {
        return false;
    }

    device_properties = device->getProperties();

    tis_log(TIS_LOG_DEBUG, "Retrieved %d properties",device_properties.size());
    bool ret = pipeline->setSource(device);

    if (ret == true)
    {
        pipeline_properties = pipeline->getFilterProperties();
    }
    
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

    pipeline->destroyPipeline();

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

    for (const auto& p : device_properties)
    {
        props.push_back(*p);
    }

    for ( const auto& p : pipeline_properties)
    {
        props.push_back(*p);
    }

    return props;
}


std::vector<VideoFormatDescription> Grabber::getAvailableVideoFormats () const
{
    if (!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return std::vector<VideoFormatDescription>();
    }

    return pipeline->getAvailableVideoFormats();
}


bool Grabber::setVideoFormat (const VideoFormat& _format)
{
    if (!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return false;
    }

    pipeline->setVideoFormat(_format);

    return this->device->setVideoFormat(_format);
}


VideoFormat Grabber::getActiveVideoFormat () const
{
    if(!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return VideoFormat();
    }
        
    return device->getActiveVideoFormat();
}


bool Grabber::startStream (std::shared_ptr<ImageSink> sink)
{
    if (!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return false;
    }
    pipeline->setSink(sink);

    return pipeline->setPipelineStatus(PIPELINE_PLAYING);
}


bool Grabber::stopStream ()
{
    if (!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return false;
    }

    return pipeline->setPipelineStatus(PIPELINE_STOPPED);
}


