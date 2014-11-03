
#include "CaptureDevice.h"

#include "Error.h"

#include "logging.h"
#include "utils.h"
#include "serialization.h"

using namespace tcam;


CaptureDevice::CaptureDevice ()
    : device(nullptr), pipeline(std::make_shared<PipelineManager>())
{}


CaptureDevice::~CaptureDevice ()
{
    if (isDeviceOpen())
        closeDevice();
}


bool CaptureDevice::load_configuration (const std::string& filename)
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


bool CaptureDevice::save_configuration (const std::string& filename)
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


bool CaptureDevice::openDevice (const DeviceInfo& device_desc)
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
    pipeline = std::make_shared<PipelineManager>();
    bool ret = pipeline->setSource(device);

    if (ret == true)
    {
        pipeline_properties = pipeline->getFilterProperties();
    }

    return true;
}


bool CaptureDevice::isDeviceOpen () const
{
    resetError();
    if (device != nullptr)
    {
        return true;
    }

    return false;
}


DeviceInfo CaptureDevice::getDevice () const
{
    return this->open_device;
}


bool CaptureDevice::closeDevice ()
{
    std::string name = open_device.getName();

    pipeline->destroyPipeline();

    open_device = DeviceInfo ();
    device.reset();
    device_properties.clear();

    tcam_log(TCAM_LOG_INFO, "Closed device %s.", name.c_str());

    return true;
}


std::vector<Property> CaptureDevice::getAvailableProperties ()
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


std::vector<VideoFormatDescription> CaptureDevice::getAvailableVideoFormats () const
{
    resetError();
    if (!isDeviceOpen())
    {
        return std::vector<VideoFormatDescription>();
    }

    return pipeline->getAvailableVideoFormats();
}


bool CaptureDevice::setVideoFormat (const VideoFormat& new_format)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    pipeline->setVideoFormat(new_format);

    return this->device->setVideoFormat(new_format);
}


VideoFormat CaptureDevice::getActiveVideoFormat () const
{
    resetError();
    if(!isDeviceOpen())
    {
        return VideoFormat();
    }

    return device->getActiveVideoFormat();
}


bool CaptureDevice::startStream (std::shared_ptr<SinkInterface> sink)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }
    pipeline->setSink(sink);

    return pipeline->setStatus(PIPELINE_PLAYING);
}


bool CaptureDevice::stopStream ()
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    return pipeline->setStatus(PIPELINE_STOPPED);
}
