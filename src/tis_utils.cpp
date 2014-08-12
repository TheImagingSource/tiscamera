
#include "tis_utils.h"

#include <sys/ioctl.h>
#include <errno.h>

using namespace tis_imaging;

std::string tis_imaging::propertyType2String (const PROPERTY_TYPE& type)
{
    switch (type)
    {
        case PROPERTY_TYPE_BOOLEAN:      return "BOOLEAN";
        case PROPERTY_TYPE_INTEGER:      return "INTEGER";
        case PROPERTY_TYPE_DOUBLE:       return "DOUBLE";
        case PROPERTY_TYPE_STRING:       return "STRING";
        case PROPERTY_TYPE_STRING_TABLE: return "STRING_TABLE";
        case PROPERTY_TYPE_BUTTON:       return "BUTTON";
        case PROPERTY_TYPE_BITMASK:      return "BITMASK";
        case PROPERTY_TYPE_UNKNOWN:
        default:
            return "";
    }
}


int tis_imaging::tis_xioctl (int fd, int request, void *arg)
{
    int r = -1;

    do
    {
        r = ioctl(fd, request, arg);
    } while (-1 == r && EINTR == errno);

    return r;
}


// unsigned int tis_get_bits_per_pixel (uint32_t fcc)
// {
//     switch (fcc)
//     {
//         case FOURCC_RGB24:      return 24;
//         case FOURCC_RGB32:      return 32;
//         case FOURCC_YUY2:       return 16;
//         case FOURCC_UYVY:       return 16;
//         case FOURCC_Y800:       return 8;
//         case FOURCC_BY8:        return 8;
//         case FOURCC_BGGR8:      return 8;
//         case FOURCC_GBRG8:      return 8;
//         case FOURCC_RGGB8:      return 8;
//         case FOURCC_GRBG8:      return 8;
//         case FOURCC_YGB0:       return 16;
//         case FOURCC_YGB1:       return 16;
//         case FOURCC_Y16  :      return 16;
//         case FOURCC_YV16:       return 16;
//         case FOURCC_I420:       return 12;
//         case FOURCC_YUV8PLANAR: return 24;
//         default:
//             return 8;
//     }
// }


// unsigned int tis_imaging::tis_get_required_buffer_size (struct tis_video_format* format)
// {
//     unsigned int byte = tis_get_bits_per_pixel (format->fourcc) / 8;
    
//     unsigned int size = format->width * format->height * byte;

//     return size;
// }
