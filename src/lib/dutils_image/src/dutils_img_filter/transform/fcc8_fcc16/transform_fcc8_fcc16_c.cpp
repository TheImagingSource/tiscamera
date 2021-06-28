
#include "transform_fcc8_fcc16.h"

namespace
{
    inline void transform_by8_to_by16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width )
    {
        for( int x = 0; x < width; x += 1 )
        {
            dst_line[x] = uint16_t( src_line[x] ) << 8;
        }
    }

    inline void transform_by16_to_by8_c_line( const uint16_t* src_line, uint8_t* dst_line, int width )
    {
        for( int x = 0; x < width; x += 1 )
        {
            dst_line[x] = uint8_t( (src_line[x] >> 8) & 0xFF );
        }
    }

    void transform_fcc8_to_fcc16_c_v0( img::img_descriptor dst, img::img_descriptor src )
    {
        for( int y = 0; y < src.dim.cy; ++y )
        {
            auto* src_line = img::get_line_start<const uint8_t>( src, y );
            auto* dst_line = img::get_line_start<uint16_t>( dst, y );

            transform_by8_to_by16_c_line( src_line, dst_line, src.dim.cx );
        }
    }


    void transform_fcc16_to_fcc8_c_v0( img::img_descriptor dst, img::img_descriptor src )
    {
        for( int y = 0; y < src.dim.cy; ++y )
        {
            auto* src_line = img::get_line_start<const uint16_t>( src, y );
            auto* dst_line = img::get_line_start<uint8_t>( dst, y );

            transform_by16_to_by8_c_line( src_line, dst_line, src.dim.cx );
        }
    }

}

img_filter::transform_function_type  img_filter::transform::get_transform_fcc8_to_fcc16_c( const img::img_type& dst, const img::img_type& src )
{
    if( dst.dim != src.dim ) {
        return nullptr;
    }
    if( can_convert_fcc8_to_fcc16( dst, src ) ) {
        return &transform_fcc8_to_fcc16_c_v0;
    }
    return nullptr;
}

img_filter::transform_function_type  img_filter::transform::get_transform_fcc16_to_fcc8_c( const img::img_type& dst, const img::img_type& src )
{
    if( dst.dim != src.dim ) {
        return nullptr;
    }
    if( can_convert_fcc16_to_fcc8( dst, src ) ) {
        return transform_fcc16_to_fcc8_c_v0;
    }
    return nullptr;
}
