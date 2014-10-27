
#include "Grabber.h"

#include "Error.h"

#include "logging.h"
#include "utils.h"
#include "serialization.h"

using namespace tcam;


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
    resetError();

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
    resetError();

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
    resetError();

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
            tcam_log(TCAM_LOG_ERROR, "Unable to close previous device.");
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
        tcam_log(TCAM_LOG_ERROR, "Device did not expose any properties!");
    }
    else
    {
        tcam_log(TCAM_LOG_DEBUG, "Retrieved %d properties", device_properties.size());
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
    resetError();
    if (device != nullptr)
    {
        return true;
    }

    tcam_log(TCAM_LOG_ERROR, "No open device");
    setError(Error("No open device", ENOENT));

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

    tcam_log(TCAM_LOG_INFO, "Closed device %s.", name.c_str());

    return true;
}


std::vector<Property> Grabber::getAvailableProperties ()
{
    resetError();
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
    resetError();
    if (!isDeviceOpen())
    {
        return std::vector<VideoFormatDescription>();
    }

    return pipeline->getAvailableVideoFormats();
}


bool Grabber::setVideoFormat (const VideoFormat& new_format)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    pipeline->setVideoFormat(new_format);

    return this->device->setVideoFormat(new_format);
}


VideoFormat Grabber::getActiveVideoFormat () const
{
    resetError();
    if(!isDeviceOpen())
    {
        return VideoFormat();
    }

    return device->getActiveVideoFormat();
}


bool Grabber::startStream (std::shared_ptr<SinkInterface> sink)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }
    pipeline->setSink(sink);

    return pipeline->setStatus(PIPELINE_PLAYING);
}


bool Grabber::stopStream ()
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    return pipeline->setStatus(PIPELINE_STOPPED);
}
