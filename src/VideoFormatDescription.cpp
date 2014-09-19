

#include "VideoFormatDescription.h"

#include "tis_logging.h"

#include <algorithm>
#include <cstring>

using namespace tis_imaging;


VideoFormatDescription::VideoFormatDescription (const struct video_format_description& _format,
                                                const std::vector<double>& _framerates)
{
    memcpy(&format, &_format, sizeof(format));
}


VideoFormatDescription::VideoFormatDescription (const struct video_format_description& f,
                                                const std::vector<res_fps>& r)
    : rf(r)
{
    memcpy(&format, &f, sizeof(format));
}


VideoFormatDescription::VideoFormatDescription (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    rf = other.rf;
}


VideoFormatDescription& VideoFormatDescription::operator= (const VideoFormatDescription& other)
{
    memcpy(&format, &other.format, sizeof(format));
    rf = other.rf;

    return *this;
}


bool VideoFormatDescription::operator== (const VideoFormatDescription& other) const
{
    // TODO: complete comparison
    return (memcmp(&format, &other.format, sizeof(format)) == 0);
}


bool VideoFormatDescription::operator!= (const VideoFormatDescription& other) const
{
    return !(*this == other);
}


struct video_format_description VideoFormatDescription::getFormatDescription () const
{
    return format;
}


std::vector<res_fps> VideoFormatDescription::getResolutionsFramesrates () const
{
    return rf;
}


std::vector<IMG_SIZE> VideoFormatDescription::getResolutions () const
{
    std::vector<IMG_SIZE> vec;

    for (auto r : rf)
    {
        vec.push_back(r.resolution);
    }

    return vec;
}


IMG_SIZE VideoFormatDescription::getSizeMin () const
{
    return format.min_size;
}


IMG_SIZE VideoFormatDescription::getSizeMax () const
{
    return format.max_size;
}


std::vector<double> VideoFormatDescription::getFrameRates (const IMG_SIZE& size) const
{

    for (auto r : rf)
    {
        if (size.width == r.resolution.width && size.height == r.resolution.height)
        {
            return r.fps;
        }
    }

    return std::vector<double>();
}


VideoFormat VideoFormatDescription::createVideoFormat (const unsigned int width,
                                                       const unsigned int height,
                                                       const double framerate) const
{

    // TODO validity check

    video_format f = {};

    f.fourcc = this->format.fourcc;
    f.width = width;
    f.height = height;
    f.binning = this->format.binning;

    return VideoFormat(f);
}


bool VideoFormatDescription::isValidVideoFormat (const VideoFormat& to_check) const
{
    auto desc = to_check.getFormatDescription();

    if (format.fourcc != desc.fourcc)
    {
        return false;
    }

    if (format.binning != desc.binning)
    {
        return false;
    }

    if (!isValidFramerate(to_check.getFramerate()))
    {
        return false;
    }


    if (!isValidResolution(to_check.getSize().width, to_check.getSize().height))
    {
        // tis_log(TIS_LOG_ERROR, "Resolution is not");
        return false;
    }

    return true;
}


bool VideoFormatDescription::isValidFramerate (const double framerate) const
{
    // auto desc = to_check.getFormatDescription();


    // TODO: framerate checking
    // if (this->format.framerate_type == TIS_FRAMERATE_TYPE_FIXED)
    // {
    // auto f = [&desc] (const double& fps)
    // {
    // return fps == desc.framerate;
    // };

    // auto res = std::find_if(framerates.begin(), framerates.end(), f);

    // if (res == framerates.end())
    // {
    // return false;
    // }
    // }
    // else
    // {
    // double min = this->framerates.at(0);
    // double max = this->framerates.at(1);

    // if (min > desc.framerate || desc.framerate > max)
    // {
    // return false;
    // }
    // }
    return true;
}


bool VideoFormatDescription::isValidResolution (const unsigned int width, const unsigned int height) const
{
    if (format.framerate_type == TIS_FRAMERATE_TYPE_FIXED)
    {
        return (format.min_size.width == width && format.min_size.height == height);
    }
    else
    {
        return (format.min_size.width <= width &&
                format.min_size.height <= height &&
                format.max_size.width >= width && format.max_size.height >= height);
    }
}
