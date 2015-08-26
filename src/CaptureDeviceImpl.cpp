
#include "CaptureDeviceImpl.h"

#include "Error.h"

#include "logging.h"
#include "utils.h"
#include "serialization.h"

using namespace tcam;


CaptureDeviceImpl::CaptureDeviceImpl ()
    : pipeline(nullptr), property_handler(nullptr), device(nullptr)
{}


CaptureDeviceImpl::CaptureDeviceImpl (const DeviceInfo& device)
    : pipeline(nullptr), property_handler(nullptr), device(nullptr)
{
    if (!openDevice(device))
    {
        tcam_log(TCAM_LOG_ERROR, "Unable to open device");
    }
}


CaptureDeviceImpl::~CaptureDeviceImpl ()
{}


bool CaptureDeviceImpl::load_configuration (const std::string& filename)
{
    resetError();

    if (!isDeviceOpen())
    {
        return false;
    }

    auto vec = property_handler->get_properties();

    return load_xml_description(filename,
                                open_device,
                                active_format,
                                vec);
}


bool CaptureDeviceImpl::save_configuration (const std::string& filename)
{
    resetError();

    if (!isDeviceOpen())
    {
        return false;
    }

    return save_xml_description(filename,
                                open_device,
                                device->get_active_video_format(),
                                property_handler->get_properties());
}


bool CaptureDeviceImpl::openDevice (const DeviceInfo& device_desc)
{
    resetError();

    if (isDeviceOpen())
    {
        bool ret = closeDevice();
        if (ret == false)
        {
            tcam_log(TCAM_LOG_ERROR, "Unable to close previous device.");
            // setError(Error("A device is already open", EPERM));
            return false;
        }
    }

    open_device = device_desc;

    device = openDeviceInterface(open_device);

    if (device == nullptr)
    {
        return false;
    }

    pipeline = std::make_shared<PipelineManager>();
    pipeline->setSource(device);

    property_handler = std::make_shared<PropertyHandler>();

    property_handler->set_properties(device->getProperties(), pipeline->getFilterProperties());

    return true;
}


bool CaptureDeviceImpl::isDeviceOpen () const
{
    resetError();
    if (device != nullptr)
    {
        return true;
    }

    return false;
}


DeviceInfo CaptureDeviceImpl::getDevice () const
{
    return this->open_device;
}


bool CaptureDeviceImpl::closeDevice ()
{
    if (!isDeviceOpen())
    {
        return true;
    }

    std::string name = open_device.get_name();

    pipeline->destroyPipeline();

    open_device = DeviceInfo ();
    device.reset();
    property_handler = nullptr;

    tcam_log(TCAM_LOG_INFO, "Closed device %s.", name.c_str());

    return true;
}


std::vector<Property*> CaptureDeviceImpl::getAvailableProperties ()
{
    resetError();
    if (!isDeviceOpen())
    {
        return std::vector<Property*>();
    }

    std::vector<Property*> props;

    for ( const auto& p : property_handler->get_properties())
    {
        props.push_back(&*p);
    }

    return props;
}


std::vector<VideoFormatDescription> CaptureDeviceImpl::getAvailableVideoFormats () const
{
    resetError();
    if (!isDeviceOpen())
    {
        return std::vector<VideoFormatDescription>();
    }

    return pipeline->getAvailableVideoFormats();
}


bool CaptureDeviceImpl::setVideoFormat (const VideoFormat& new_format)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    pipeline->setVideoFormat(new_format);

    return this->device->set_video_format(new_format);
}


VideoFormat CaptureDeviceImpl::getActiveVideoFormat () const
{
    resetError();
    if(!isDeviceOpen())
    {
        return VideoFormat();
    }

    return device->get_active_video_format();
}


bool CaptureDeviceImpl::startStream (std::shared_ptr<SinkInterface> sink)
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }
    pipeline->setSink(sink);

    return pipeline->set_status(TCAM_PIPELINE_PLAYING);
}


bool CaptureDeviceImpl::stopStream ()
{
    resetError();
    if (!isDeviceOpen())
    {
        return false;
    }

    return pipeline->set_status(TCAM_PIPELINE_STOPPED);
}
