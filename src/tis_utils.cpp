
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
