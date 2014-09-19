

#include "VideoFormat.h"
#include "tis_logging.h"
#include "tis_utils.h"

#include <iomanip>              // setprecision
#include <sstream>
#include <cstring>
#include <cstdlib>

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


bool VideoFormat::operator== (const VideoFormat& other) const
{
    return format.fourcc == other.format.fourcc
        && format.width == other.format.width
        && format.height == other.format.height
        && format.binning == other.format.binning;

    // TODO: framerate
}


bool VideoFormat::operator!= (const VideoFormat& other) const
{
    return !(*this == other);
}


struct video_format VideoFormat::getFormatDescription () const
{
    return format;
}


uint32_t VideoFormat::getFourcc () const
{
    return format.fourcc;
}


void VideoFormat::setFourcc (const uint32_t& fourcc)
{
    format.fourcc = fourcc;
}


double VideoFormat::getFramerate () const
{
    return format.framerate;
}


void VideoFormat::setFramerate (const double& framerate)
{
    format.framerate = framerate;
}


struct IMG_SIZE VideoFormat::getSize () const
{
    IMG_SIZE s = {format.width, format.height};
    return s;
}


void VideoFormat::setSize (const unsigned int& width, const unsigned int& height)
{
    format.width = width;
    format.height = height;
}


unsigned int VideoFormat::getBinning () const
{
    return format.binning;
}


void VideoFormat::setBinning (const unsigned int binning)
{
    format.binning = binning;
}


std::string VideoFormat::toString () const
{
    std::string s;

    s  = "format=";//    +
    //s.append(fourcc2description(format.fourcc));
    s += std::to_string(format.fourcc);
        s += ",";
    s += "width="     + std::to_string(format.width)   + ",";
    s += "height="    + std::to_string(format.height)  + ",";
    s += "binning="   + std::to_string(format.binning) + ",";

    std::ostringstream out;
    // out.precision(6);
    // << std::setprecision(15)
    // out << std::setprecision(15) << format.framerate;
    // return out.str();
    // s += "framerate=" +  out.str();
    
    s += "framerate=" + std::to_string(format.framerate);
        
    return s;
}


bool VideoFormat::fromString (const std::string& desc)
{
    video_format f = {};

    auto vec = split_string(desc, ",");

    for (auto v : vec)
    {
        auto val = split_string(v, "=");

        if (val.size() != 2)
        {
            tis_log(TIS_LOG_ERROR, "Received faulty VideoFormat String \"%s\"", v.c_str());
            return false;
        }
        
        if (val[0].compare("format") == 0)
        {
            // TODO: different format definitions
            // union
            // {
            //     uint32_t i;
            //     char c[4];
            // } f;

            // f.c = val[1].c_str();
            
            // f.fourcc = stoi(val[1]);
            // f.fourcc = string2fourcc(val[1]);
            tis_log(TIS_LOG_ERROR, "format is  \"%s\"", val[1].c_str());

            f.fourcc = std::stoul(val[1]);
        }
        else if (val[0].compare("width") == 0)
        {
            f.width = stoi(val[1]);
        }
        else if (val[0].compare("height") == 0)
        {
            f.height = stoi(val[1]);
        }
        else if (val[0].compare("binning") == 0)
        {
            f.binning = stoi(val[1]);
        }
        else if (val[0].compare("framerate") == 0)
        {
            f.framerate = stod(val[1]);
        }
        else
        {
            tis_log(TIS_LOG_ERROR, "Unknown descriptor in VideoFormat String \"%s\"", val[0].c_str());
            return false;
        }
    }

    this->format = f;
    
    return true;
}
    
