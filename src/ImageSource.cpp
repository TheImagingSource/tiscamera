
#include "ImageSource.h"

#include "tis_logging.h"

using namespace tis_imaging;

ImageSource::ImageSource ()
    : current_status(PIPELINE_UNDEFINED), n_buffers(10)
{}


ImageSource::~ImageSource ()
{
    device->release_buffers();
}


bool ImageSource::setStatus (PIPELINE_STATUS status)
{
    current_status = status;

    if (current_status == PIPELINE_PLAYING)
    {
        this->initialize_buffers();

        device->initialize_buffers(buffers);
        device->setSink(shared_from_this());

        //tis_log(TIS_LOG_ERROR, "Source changed to state PLAYING");
        if ( device->start_stream())
        {
            tis_log(TIS_LOG_DEBUG, "PLAYING....");
        }
        else
        {
            tis_log(TIS_LOG_ERROR, "Unable to start stream from device.");
            return false;
        }

    }
    else if (current_status == PIPELINE_STOPPED)
    {
        tis_log(TIS_LOG_INFO, "Source changed to state STOPPED");
        device->stop_stream();
        device->release_buffers();
    }


    return true;
}


PIPELINE_STATUS ImageSource::getStatus () const
{
    return current_status;
}


bool ImageSource::setDevice (std::shared_ptr<DeviceInterface> dev)
{
    //tis_log(TIS_LOG_DEBUG, "Received device to use as source.");

    if (current_status == PIPELINE_PAUSED || current_status == PIPELINE_PLAYING)
    {
        return false;
    }

    device = dev;

    //tis_log(TIS_LOG_INFO, "Giving %d buffers to initialize in device", buffers.size());

    if (buffers.empty())
    {
        tis_log(TIS_LOG_ERROR, "Have no Buffers to work with");
        return false;
    }


    return true;
}


bool ImageSource::setVideoFormat (const VideoFormat& f)
{
    return device->setVideoFormat(f);
}


VideoFormat ImageSource::getVideoFormat () const
{
    return device->getActiveVideoFormat();
}


void ImageSource::pushImage (std::shared_ptr<MemoryBuffer> buffer)
{

    // tis_log(TIS_LOG_INFO, "received buffer");

    if (!pipeline.expired())
        pipeline.lock()->pushImage(buffer);
    else
        tis_log(TIS_LOG_ERROR, "Pipeline over expiration date.");
}


void ImageSource::initialize_buffers ()
{
    VideoFormat f = device->getActiveVideoFormat();

    struct video_format format = f.getFormatDescription();
    int bit_depth = img::getBitsPerPixel(format.fourcc);

    for (unsigned int i = 0; i < this->n_buffers; ++i)
    {
        struct image_buffer b = {};

        b.pData = NULL;
        b.length = format.width * format.height * bit_depth;
        b.format = format;
        b.pitch = format.width * bit_depth / 8;

        auto ptr = std::make_shared<MemoryBuffer>(MemoryBuffer(b));

        this->buffers.push_back(ptr);
    }
}


bool ImageSource::setSink (std::shared_ptr<SinkInterface> sink)
{
    this->pipeline = sink;

    return true;
}
