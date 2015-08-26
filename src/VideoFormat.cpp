

#include "VideoFormat.h"
#include "logging.h"
#include "utils.h"

#include <iomanip>              // setprecision
#include <sstream>
#include <cstring>
#include <cstdlib>

using namespace tcam;


VideoFormat::VideoFormat ()
    :format()
{}


VideoFormat::VideoFormat (const struct tcam_video_format& new_format)
{
    memcpy(&format, &new_format, sizeof(format));
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
        && format.height == other.format.height;
        //&& compare_double(format.framerate, other.format.framerate);
}


bool VideoFormat::operator!= (const VideoFormat& other) const
{
    return !(*this == other);
}


struct tcam_video_format VideoFormat::getStruct () const
{
    return format;
}


uint32_t VideoFormat::getFourcc () const
{
    return format.fourcc;
}


void VideoFormat::setFourcc (uint32_t fourcc)
{
    format.fourcc = fourcc;
}


double VideoFormat::getFramerate () const
{
    return format.framerate;
}


void VideoFormat::setFramerate (double framerate)
{
    format.framerate = framerate;
}


struct tcam_image_size VideoFormat::getSize () const
{
    tcam_image_size s = {format.width, format.height};
    return s;
}


void VideoFormat::setSize (unsigned int width, unsigned int height)
{
    format.width = width;
    format.height = height;
}


std::string VideoFormat::toString () const
{
    std::string s;

    s  = "format=";
    s += fourcc2description(format.fourcc);
    s += ",";
    s += "width="     + std::to_string(format.width)   + ",";
    s += "height="    + std::to_string(format.height)  + ",";
    s += "framerate=" + std::to_string(format.framerate);

    return s;
}


bool VideoFormat::fromString (const std::string& desc)
{
    tcam_video_format f = {};

    auto vec = split_string(desc, ",");

    for (auto v : vec)
    {
        auto val = split_string(v, "=");

        if (val.size() != 2)
        {
            tcam_log(TCAM_LOG_ERROR, "Received faulty VideoFormat String \"%s\"", v.c_str());
            return false;
        }

        if (val[0].compare("format") == 0)
        {
            tcam_log(TCAM_LOG_ERROR, "format is  \"%s\"", val[1].c_str());

            f.fourcc  = description2fourcc(val[1].c_str());
        }
        else if (val[0].compare("width") == 0)
        {
            f.width = stoi(val[1]);
        }
        else if (val[0].compare("height") == 0)
        {
            f.height = stoi(val[1]);
        }
        else if (val[0].compare("framerate") == 0)
        {
            f.framerate = stod(val[1]);
        }
        else
        {
            tcam_log(TCAM_LOG_ERROR, "Unknown descriptor in VideoFormat String \"%s\"", val[0].c_str());
            return false;
        }
    }

    this->format = f;

    return true;
}


uint64_t VideoFormat::getRequiredBufferSize () const
{
    return get_buffer_length(format.width, format.height, format.fourcc);
}


uint32_t VideoFormat::getPitchSize () const
{
    return get_pitch_length(format.width, format.fourcc);
}
