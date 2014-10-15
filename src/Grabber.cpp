
#include "Grabber.h"

#include "tis_logging.h"
#include "tis_utils.h"
#include "serialization.h"

using namespace tis_imaging;


Grabber::Grabber ()
    : device(nullptr), pipeline(std::make_shared<PipelineManager>())
{}


Grabber::~Grabber ()
{
    if (isDeviceOpen())
        closeDevice();
}


bool Grabber::load_configuration (const std::string& filename)
{
    if (!isDeviceOpen())
    {
        return false;
    }

    std::vector<std::shared_ptr<Property>> properties ;
    properties.reserve(device_properties.size() + pipeline_properties.size());

    properties.insert(properties.end(), device_properties.begin(), device_properties.end());
    properties.insert(properties.end(), pipeline_properties.begin(), pipeline_properties.end());

    return load_xml_description(filename,
                                open_device,
                                active_format,
                                properties);
}


bool Grabber::save_configuration (const std::string& filename)
{
    if (!isDeviceOpen())
    {
        return false;
    }

    std::vector<std::shared_ptr<Property>> properties ;
    properties.reserve(device_properties.size() + pipeline_properties.size());

    properties.insert(properties.end(), device_properties.begin(), device_properties.end());
    properties.insert(properties.end(), pipeline_properties.begin(), pipeline_properties.end());

    return save_xml_description(filename,
                                open_device,
                                device->getActiveVideoFormat(),
                                properties);
}


bool Grabber::openDevice (const CaptureDevice& device_desc)
{
    if (pipeline->getStatus() == PIPELINE_PLAYING ||
        pipeline->getStatus() == PIPELINE_PAUSED)
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

    open_device = device_desc;

    device = openDeviceInterface(open_device);

    if (device == nullptr)
    {
        return false;
    }

    device_properties = device->getProperties();

    if (device_properties.empty())
    {
        tis_log(TIS_LOG_ERROR, "Device did not expose any properties!");
    }
    else
    {
        tis_log(TIS_LOG_DEBUG, "Retrieved %d properties", device_properties.size());
    }
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


bool Grabber::setVideoFormat (const VideoFormat& new_format)
{
    if (!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return false;
    }

    pipeline->setVideoFormat(new_format);

    return this->device->setVideoFormat(new_format);
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


bool Grabber::startStream (std::shared_ptr<SinkInterface> sink)
{
    if (!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return false;
    }
    pipeline->setSink(sink);

    return pipeline->setStatus(PIPELINE_PLAYING);
}


bool Grabber::stopStream ()
{
    if (!isDeviceOpen())
    {
        tis_log(TIS_LOG_ERROR, "No open device");
        return false;
    }

    return pipeline->setStatus(PIPELINE_STOPPED);
}
