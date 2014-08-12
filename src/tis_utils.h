
#ifndef _TIS_UTILS_H_
#define _TIS_UTILS_H_

// #include "tis_video.h"

#include "base_types.h"

#include <string>

namespace tis_imaging
{

std::string propertyType2String (const PROPERTY_TYPE&);

inline std::string fourcc2string (const uint32_t& fourcc)
{
    union _b
    {
        uint32_t i;
        char c[4];
    } conversion;

    conversion.i = fourcc;

    std::string s = conversion.c;
    
    return s;
}


int tis_xioctl (int fd, int request, void* arg);

unsigned int tis_get_required_buffer_size (struct tis_video_format* format);

} /* namespace tis_imaging */

#endif /* _TIS_UTILS_H_ */
