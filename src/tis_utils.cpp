
#include "tis_utils.h"

#include "tis_logging.h"

#include <cstring>
#include <sys/ioctl.h>
#include <errno.h>

#define IOCTL_RETRY 4


using namespace tis_imaging;

std::string tis_imaging::propertyType2String (const PROPERTY_TYPE& type)
{
    switch (type)
    {
        case PROPERTY_TYPE_BOOLEAN: return "BOOLEAN";
        case PROPERTY_TYPE_INTEGER: return "INTEGER";
        case PROPERTY_TYPE_DOUBLE: return "DOUBLE";
        case PROPERTY_TYPE_STRING: return "STRING";
        case PROPERTY_TYPE_STRING_TABLE: return "STRING_TABLE";
        case PROPERTY_TYPE_BUTTON: return "BUTTON";
        case PROPERTY_TYPE_BITMASK: return "BITMASK";
        case PROPERTY_TYPE_UNKNOWN:
        default:
            return "";
    }
}


std::string tis_imaging::fourcc2string (const uint32_t& fourcc)
{


    union _bla
    {
        uint32_t i;
        char c[4];
    } bla;

    bla.i = fourcc;

    std::string s (bla.c);

    // std::string s ( (char*)&fourcc);
    // s += "\0";
    return s;
}


uint32_t tis_imaging::string2fourcc (const std::string& s)
{
    if(s.length() != 4)
    {
        return 0;
    }

    uint32_t fourcc = 0;

    char c[4];

    strncpy(c, s.c_str(), 4);

    fourcc = mmioFOURCC(c[0],c[1],c[2],c[3]);

    return fourcc;
}


std::vector<std::string> tis_imaging::split_string (const std::string& to_split, const std::string &delim)
{
    std::vector<std::string> vec;

    size_t beg = 0;
    size_t end = 0;

    while (end != std::string::npos)
    {
        end = to_split.find_first_of(delim, beg);

        std::string s = to_split.substr(beg, end - beg);

        vec.push_back(s);

        beg = end + delim.size();
    }

    return vec;
}


int tis_imaging::tis_xioctl (int fd, int request, void *arg)
{
    int ret = 0;
    int tries= IOCTL_RETRY;
    do
    {
        ret = ioctl(fd, request, arg);
        // ret = v4l2_ioctl(fd, request, arg);
    }
    while (ret && tries-- &&
           ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

    if (ret && (tries <= 0))
    {
        tis_log(TIS_LOG_ERROR,"ioctl (%i) retried %i times - giving up: %s)\n", request, IOCTL_RETRY, strerror(errno));
    }

    return (ret);

}


struct fourcc_names
{
    uint32_t fourcc;
    char name[24];
};


fourcc_names fourcc_name_array [] =
{
    {FOURCC_RGB8, "RGB8"},
    {FOURCC_RGB24, "RGB24"},
    {FOURCC_RGB32, "RGB32"},
    {FOURCC_RGB64, "RGB64"},

    {FOURCC_YUY2, "YUY2"},
    {FOURCC_Y800, "Y800"},
    {FOURCC_BY8,   "BY8"},
    {FOURCC_UYVY, "UYVY"},
    {FOURCC_YGB0, "YGB0"},
    {FOURCC_YGB1, "YGB1"},
    {FOURCC_Y16, "Y16"},

    {FOURCC_Y444, "Y444"},
    {FOURCC_Y411, "Y411"},

    {FOURCC_BGGR8, "BA81"},
    {FOURCC_GBRG8, "GBRG"},
    {FOURCC_GRBG8, "GRBG"},
    {FOURCC_RGGB8, "RGGB"},

    {FOURCC_BGGR10, "BG10"},
    {FOURCC_GBRG10, "GB10"},
    {FOURCC_GRBG10, "BA10"},
    {FOURCC_RGGB10, "RG10"},

    {FOURCC_BGGR12, "BG12"},
    {FOURCC_GBRG12, "GB12"},
    {FOURCC_GRBG12, "BA12"},
    {FOURCC_RGGB12, "RG12"},
    {FOURCC_BGGR16, "BG16"},
    {FOURCC_GBRG16, "GB16"},
    {FOURCC_GRBG16, "BA16"},
    {FOURCC_RGGB16, "RG16"},

    {FOURCC_I420, "I420"},
    {FOURCC_YV16, "YV16"},
    {FOURCC_YUV8PLANAR, "YUV8 planar"},
    {FOURCC_YUV16PLANAR, "YUV16 planar"},
    {FOURCC_YUV8, "YUV8"},
    {FOURCC_H264, "H264"},
    {FOURCC_MJPG, "MJPG"},
};


const char* tis_imaging::fourcc2description (const uint32_t& fourcc)
{
    for (const auto& entry : fourcc_name_array)
    {
        if (entry.fourcc == fourcc)
        {
            return entry.name;
        }
    }
    return "";
}


uint32_t tis_imaging::description2fourcc (const char* description)
{
    for (const auto& entry : fourcc_name_array)
    {
        if (strcmp(entry.name, description) == 0)
        {
            return entry.fourcc;
        }
    }
    return 0;
}


std::vector<double> tis_imaging::createStepsForRange (double min, double max)
{
    std::vector<double> vec;

    if (max >= min)
        return vec;

    double current_step = min;

    while (current_step < max)
    {
        vec.push_back(current_step);

        if (current_step < 10.0)
        {
            current_step += 1;
        }
        else if (current_step < 20.0)
        {
            current_step += 2.0;
        }
        else
        {
            current_step += 5.0;
        }
    }

    if (vec.back() != max)
    {
        vec.push_back(max);
    }
    return vec;
}


uint64_t tis_imaging::getBufferLength (unsigned int width, unsigned int height, uint32_t fourcc)
{
    uint64_t size = width * height * (img::getBitsPerPixel(fourcc) / 8);

    return size;
}
