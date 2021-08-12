
#include "transform_mono_to_bgr.h"

#include "transform_mono_to_bgr_internal.h"

#include <dutils_img/pixel_structs.h>

#include "../../simd_helper/use_simd_A64.h"

namespace
{
    using namespace transform_mono_to_bgr_internal;
    using namespace img::pixel_type;

void	transform_MONO8_to_BGR24_neon( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( src.fourcc_type() == img::fourcc::MONO8 && dst.fourcc_type() == img::fourcc::BGR24 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGR24>( dst, y );

        int x = 0;
        for( x = 0; x < (dst.dim.cx - 7); x += 8 )
        {
            uint8x8_t pix_val = vld1_u8( src_line + x );

            vst3_u8( reinterpret_cast<uint8_t*>( dst_line + x ), uint8x8x3_t{ pix_val, pix_val, pix_val } );
        }
        transform_MONO8_to_BGR24_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}

void	transform_MONO8_to_BGRA32_neon( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( src.fourcc_type() == img::fourcc::MONO8 && dst.fourcc_type() == img::fourcc::BGRA32  );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA32>( dst, y );

        int x = 0;
        for( x = 0; x < (dst.dim.cx - 7); x += 8 )
        {
            uint8x8_t pix_val = vld1_u8( src_line + x );

            vst4_u8( reinterpret_cast<uint8_t*>(dst_line + x), uint8x8x4_t{ pix_val, pix_val, pix_val, 0xFF } );
        }
        transform_MONO8_to_BGRA32_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}


void	transform_MONO16_to_BGRA32_neon( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( src.fourcc_type() == img::fourcc::MONO16 && dst.fourcc_type() == img::fourcc::BGRA32 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA32>( dst, y );

        int x = 0;
        for( x = 0; x < (dst.dim.cx - 7); x += 8 )
        {
            uint16x8_t read_val = vld1q_u16( src_line + x );
            auto tmp0 = vshrn_n_u16( read_val, 8 );

            vst4_u8( reinterpret_cast<uint8_t*>(dst_line + x), uint8x8x4_t{ tmp0, tmp0, tmp0, 0xFF } );
        }
        transform_MONO16_to_BGRA32_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}

void	transform_MONO16_to_BGRA64_neon( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( src.fourcc_type() == img::fourcc::MONO16 && dst.fourcc_type() == img::fourcc::BGRA64 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA64>( dst, y );

        int x = 0;
        for( x = 0; x < (dst.dim.cx - 7); x += 8 )
        {
            uint16x8_t tmp0 = vld1q_u16( src_line + x );

            vst4q_u16( reinterpret_cast<uint16_t*>(dst_line + x), uint16x8x4_t{ tmp0, tmp0, tmp0, 0xFF } );
        }
        transform_MONO16_to_BGRA64_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}

}

img_filter::transform_function_type     img_filter::transform::get_transform_mono_to_bgr_neon( const img::img_type& dst, const img::img_type& src )
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }

    if( src.fourcc_type() == img::fourcc::MONO8 )
    {
        switch( dst.fourcc_type() )
        {
        case img::fourcc::BGR24:        return transform_MONO8_to_BGR24_neon;
        case img::fourcc::BGRA32:       return transform_MONO8_to_BGRA32_neon;
        //case img::fourcc::BGRA64:       return transform_MONO8_to_BGRA64_ssse3;
        default:
            return nullptr;
        }
    }
    if( src.fourcc_type() == img::fourcc::MONO16 )
    {
        switch( dst.fourcc_type() )
        {
        //case img::fourcc::BGR24:        return transform_MONO8_to_BGR24_neon;
        case img::fourcc::BGRA32:       return transform_MONO16_to_BGRA32_neon;
        case img::fourcc::BGRA64:       return transform_MONO16_to_BGRA64_neon;
        default:
            return nullptr;
        }
    }
    return nullptr;
}
