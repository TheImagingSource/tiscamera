


#ifndef DUTILS_HEADER
#define DUTILS_HEADER

#include <auto_alg/auto_alg.h>
#include <by8/by8_apply_whitebalance.h>

img::img_descriptor to_img_desc (MemoryBuffer& buf)
{
    image_buffer b = buf.getImageBuffer();
    img::img_descriptor img = {b.pData,
                               b.length,
                               b.format.fourcc,
                               b.format.width,
                               b.format.height,
                               b.pitch};
    
    return img;
}

#endif /* DUTILS_HEADER */










