

#pragma once

#include "transform_fcc8_fcc16.h"

namespace img_filter::transform::internal
{
    inline void transform_fcc8_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width )
    {
        for( int x = 0; x < width; x += 1 )
        {
            dst_line[x] = uint16_t( src_line[x] ) << 8;
        }
    }

    inline void transform_fcc16_to_fcc8_c_line( const uint16_t* src_line, uint8_t* dst_line, int width )
    {
        for( int x = 0; x < width; x += 1 )
        {
            dst_line[x] = uint8_t( (src_line[x] >> 8) & 0xFF );
        }
    }
}
