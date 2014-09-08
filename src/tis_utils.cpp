
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


const char* tis_imaging::fourcc2description (const uint32_t& fourcc)
{
    switch (fourcc)
    {
        // these are pseudo fourcc's used in the library to signify the formats
        case FOURCC_RGB8:
			return "RGB8";
        case FOURCC_RGB24:
            return "RGB24";
        case FOURCC_RGB32:
            return "RGB32";
        case FOURCC_RGB64:
            return "RGB64";

        case FOURCC_YUY2:
			return "YUY2";
        case FOURCC_Y800:
			return "Y800";
        case FOURCC_BY8:
			return "BY8";
        case FOURCC_UYVY:
			return "UYVY";

        case FOURCC_YGB0:
			return "YGB0";
        case FOURCC_YGB1:
			return "YGB1";
        case FOURCC_Y16:
			return "Y16";

        case FOURCC_Y444:
			return "Y444";  // TIYUV: 1394 conferencing camera 4:4:4 mode 0, 24 bit
        case FOURCC_Y411:
			return "Y411";  // TIYUV: 1394 conferencing camera 4:1:1 mode 2, 12 bit
            // case FOURCC_B800:
			// return "BY8";  
            // case FOURCC_Y422:
            // 	FOURCC_UYVY

        case FOURCC_BGGR8:
            return "BA81"; /*  8  BGBG.. GRGR.. */
        case FOURCC_GBRG8:
            return "GBRG"; /*  8  GBGB.. RGRG.. */
        case FOURCC_GRBG8:
            return "GRBG"; /*  8  GRGR.. BGBG.. */
        case FOURCC_RGGB8:
            return "RGGB"; /*  8  RGRG.. GBGB.. */

        case FOURCC_BGGR10:
            return "BG10"; /* 10  BGBG.. GRGR.. */
        case FOURCC_GBRG10:
            return "GB10"; /* 10  GBGB.. RGRG.. */
        case FOURCC_GRBG10:
            return "BA10"; /* 10  GRGR.. BGBG.. */
        case FOURCC_RGGB10:
            return "RG10"; /* 10  RGRG.. GBGB.. */

        case FOURCC_BGGR12:
            return "BG12"; /* 12  BGBG.. GRGR.. */
        case FOURCC_GBRG12:
            return "GB12"; /* 12  GBGB.. RGRG.. */
        case FOURCC_GRBG12:
            return "BA12"; /* 12  GRGR.. BGBG.. */
        case FOURCC_RGGB12:
            return "RG12"; /* 12  RGRG.. GBGB.. */

        case FOURCC_BGGR16:
            return "BG16"; /* 16  BGBG.. GRGR.. */
        case FOURCC_GBRG16:
            return "GB16"; /* 16  GBGB.. RGRG.. */
        case FOURCC_GRBG16:
            return "BA16"; /* 16  GRGR.. BGBG.. */
        case FOURCC_RGGB16:
            return "RG16"; /* 16  RGRG.. GBGB.. */

        case FOURCC_I420:
			return "I420"; 
        case FOURCC_YV16:
			return "YV16";		// YUV planar Y plane, U plane, V plane. U and V sub sampled in horz 
//case FOURCC_YV12			return "Y', 'V', '1', '2"; 
        case FOURCC_YUV8PLANAR:
            return "YUV8 planar";		// unofficial, YUV planar, Y U V planes, all 8 bit, no sub-sampling
        case FOURCC_YUV16PLANAR:
            return "YUV16 planar";		// unofficial, YUV planar, Y U V planes, all 16 bit, no sub-sampling
        case FOURCC_YUV8:
			return "YUV8";      // 8 bit, U Y V _ ordering, 32 bit

        case FOURCC_H264:
            return "H264";
        case FOURCC_MJPG:
            return "MJPG";
        default:
            return "";
    }
}
