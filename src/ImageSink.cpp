

#include "ImageSink.h"

#include <iostream>

using namespace tis_imaging;


ImageSink::ImageSink ()
    : status(PIPELINE_UNDEFINED), callback(nullptr), user_data(nullptr)
{}


ImageSink::~ImageSink ()
{}


bool ImageSink::setStatus (const PIPELINE_STATUS& s)
{
    if (status == s)
        return true;

    status = s;

    if (status == PIPELINE_PLAYING)
    {
        std::cout << "Pipeline started playing" << std::endl;
    }
    else if (status == PIPELINE_STOPPED)
    {
        std::cout << "Pipeline stopped playing" << std::endl;
    }

    return true;
}


PIPELINE_STATUS ImageSink::getStatus () const
{
    return status;
}


bool ImageSink::registerCallback (sink_callback sc, void* ud)
{
    this->callback = sc;
    this->user_data = ud;

    return true;
}



void ImageSink::pushImage (std::shared_ptr<MemoryBuffer> buffer)
{
    if (callback != nullptr)
    {
        this->callback(buffer, user_data);
    }
}
