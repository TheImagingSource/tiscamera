
#ifndef TCAM_DUTILS_HEADER_H
#define TCAM_DUTILS_HEADER_H

#include <auto_alg/auto_alg.h>
#include <by8/by8_apply_whitebalance.h>
#include "tcam.h"

// #include <MemoryBuffer.h>
// #include <base_types.h>

// namespace tcam
// {


inline img::img_descriptor to_img_desc (const tcam_image_buffer& buf)
{
    img::img_descriptor img = {};
    img.pData = buf.pData;
    img.pitch = buf.pitch;
    img.data_length = buf.length;
    img.type = buf.format.fourcc;
    img.dim_x = buf.format.width;
    img.dim_y = buf.format.height;

    return img;
}


// inline img::img_descriptor to_img_desc (tcam::MemoryBuffer& buf)
// {
//     auto b = buf.getImageBuffer();
//     return to_img_desc(b);
// }


// } /* namespace tcam */

#endif /* TCAM_DUTILS_HEADER_H */
