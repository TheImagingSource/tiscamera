

#include "VideoFormatDescription.h"

#include "logging.h"
#include "utils.h"

#include <algorithm>
#include <cstring>

using namespace tcam;


VideoFormatDescription::VideoFormatDescription (const struct tcam_video_format_description& f,
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


struct tcam_video_format_description VideoFormatDescription::getFormatDescription () const
{
    return format;
}


uint32_t VideoFormatDescription::getFourcc () const
{
    return format.fourcc;
}


std::vector<res_fps> VideoFormatDescription::getResolutionsFramesrates () const
{
    return rf;
}


std::vector<tcam_image_size> VideoFormatDescription::getResolutions () const
{
    std::vector<tcam_image_size> vec;

    for (auto r : rf)
    {
        vec.push_back(r.resolution);
    }

    return vec;
}


tcam_image_size VideoFormatDescription::getSizeMin () const
{
    return format.min_size;
}


tcam_image_size VideoFormatDescription::getSizeMax () const
{
    return format.max_size;
}


std::vector<double> VideoFormatDescription::getFrameRates (const tcam_image_size& size) const
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


VideoFormat VideoFormatDescription::createVideoFormat (unsigned int width,
                                                       unsigned int height,
                                                       double framerate) const
{

    // TODO validity check

    tcam_video_format f = {};

    f.fourcc = this->format.fourcc;
    f.width = width;
    f.height = height;
    f.binning = this->format.binning;
    f.framerate = framerate;

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
        // tcam_log(TCAM_LOG_ERROR, "Resolution is not");
        return false;
    }

    return true;
}


bool VideoFormatDescription::isValidFramerate (double framerate) const
{
    // auto desc = to_check.getFormatDescription();

    for (const auto& res: rf)
    {
        if (format.framerate_type == TCAM_FRAMERATE_TYPE_FIXED)
        {
            for (const auto& f : res.fps)
            {
                if (compare_double(framerate, f))
                {
                    return true;
                }
            }
        }
        else // range
        {
            if (framerate <= res.fps.at(0) && framerate >= res.fps.at(1))
            {
                return true;
            }
        }
    }

    return false;

}


bool VideoFormatDescription::isValidResolution (unsigned int width, unsigned int height) const
{
    if (format.framerate_type == TCAM_FRAMERATE_TYPE_FIXED)
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
