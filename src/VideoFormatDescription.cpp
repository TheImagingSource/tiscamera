

#include "VideoFormatDescription.h"

#include "tis_logging.h"

#include <algorithm>
#include <cstring>

using namespace tis_imaging;


VideoFormatDescription::VideoFormatDescription (const struct video_format_description& _format,
                                                const std::vector<double>& _framerates)
    : framerates(_framerates)
{
    memcpy(&format, &_format, sizeof(format));
}


VideoFormatDescription::VideoFormatDescription (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    framerates = other.framerates;
}


VideoFormatDescription& VideoFormatDescription::operator= (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    framerates = other.framerates;

    return *this;
}


bool VideoFormatDescription::operator== (const VideoFormatDescription& other)
{
    return (memcmp(&format, &other.format, sizeof(format)) == 0) &&
        (framerates == framerates);
}


bool VideoFormatDescription::operator!= (const VideoFormatDescription& other)
{
    return !(*this == other);
}



VideoFormatDescription::~VideoFormatDescription ()
{}


struct video_format_description VideoFormatDescription::getFormatDescription () const
{
    return format;
}


SIZE VideoFormatDescription::getSizeMin () const
{
    return format.min_size;
}


SIZE VideoFormatDescription::getSizeMax () const
{
    return format.max_size;
}


std::vector<double> VideoFormatDescription::getFrameRates () const
{
    return framerates;
}


bool VideoFormatDescription::isValidVideoFormat (const VideoFormat& to_check) const
{
    auto desc = to_check.getFormatDescription();

    if (this->format.framerate_type == TIS_FRAMERATE_TYPE_FIXED)
    {
        auto f = [&desc] (const double& fps) { return fps == desc.framerate;};
        
        auto res = std::find_if(framerates.begin(), framerates.end(), f);
        
        if (res == framerates.end())
        {
            return false;
        }
    }
    else
    {
        double min = this->framerates.at(0);
        double max = this->framerates.at(1);
        
        if (min > desc.framerate > max)
        {
            return false;
        }
    }
    
    return true;
}

