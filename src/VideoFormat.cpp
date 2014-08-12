

#include "VideoFormat.h"

#include <cstring>

using namespace tis_imaging;


VideoFormat::VideoFormat ()
    :format()
{}


VideoFormat::VideoFormat (const struct video_format& _format)
{
    memcpy(&format, &_format, sizeof(format));
}


VideoFormat::VideoFormat (const VideoFormat& other)
{
    memcpy(&format, &other.format, sizeof(format));
}


VideoFormat& VideoFormat::operator= (const VideoFormat&  other)
{
    memcpy(&format, &other.format, sizeof(format));

    return *this;
}


VideoFormat::~VideoFormat ()
{}


struct video_format VideoFormat::getFormatDescription () const
{
    return format;
}


uint32_t VideoFormat::getFourcc () const
{
    return format.fourcc;
}


struct SIZE VideoFormat::getSize () const
{
    SIZE s = {format.width, format.height};
    return s;
}
    
