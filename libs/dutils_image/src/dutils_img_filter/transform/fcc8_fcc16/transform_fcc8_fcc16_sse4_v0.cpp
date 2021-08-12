
#include "transform_fcc8_fcc16_internal.h"

#include "../../simd_helper/use_simd_sse41.h"

#include "../src/dutils_img_base/alignment_helper.h"

using namespace simd::sse;

namespace
{

template<mem_access dst_aligned>
void transform_fcc8_to_fcc16_sse4_1_impl( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;
        for( ; x < (dst.dim.cx - 15); x += 16 )
        {
            auto reg = load_si128u( src_line + x );

            auto lo = _mm_unpacklo_epi8( _mm_setzero_si128(), reg );
            auto hi = _mm_unpackhi_epi8( _mm_setzero_si128(), reg );

            store<dst_aligned>( dst_line + x + 0, lo );
            store<dst_aligned>( dst_line + x + 8, hi );
        }
        img_filter::transform::internal::transform_fcc8_to_fcc16_c_line( src_line + x, dst_line + x, src.dim.cx - x );
    }
}


template<mem_access dst_aligned>
void transform_fcc16_to_fcc8_sse4_1_impl( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );

        int x = 0;
        for( ; x < (dst.dim.cx - 15); x += 16 )
        {
            auto reg_lo = load_si128u( src_line + x + 0 );
            auto reg_hi = load_si128u( src_line + x + 8 );

            auto v0 = _mm_srli_epi16( reg_lo, 8 );
            auto v1 = _mm_srli_epi16( reg_hi, 8 );

            auto lo = _mm_packus_epi16( v0, v1 );

            store<dst_aligned>( dst_line + x + 0, lo );
        }
        img_filter::transform::internal::transform_fcc16_to_fcc8_c_line( src_line + x, dst_line + x, src.dim.cx - x );
    }
}


static void transform_fcc8_to_fcc16_sse4_1_v0( img::img_descriptor dst, img::img_descriptor src )
{
    if( simd::is_aligned_for_sse_stream( dst ) ) {
        transform_fcc8_to_fcc16_sse4_1_impl<mem_access::stream>( dst, src );
    } else {
        transform_fcc8_to_fcc16_sse4_1_impl<mem_access::unaligned>( dst, src );
    }
}

void transform_fcc16_to_fcc8_sse4_1_v0( img::img_descriptor dst, img::img_descriptor src )
{
    if( simd::is_aligned_for_sse_stream( dst ) ) {
        transform_fcc16_to_fcc8_sse4_1_impl<mem_access::stream>( dst, src );
    } else {
        transform_fcc16_to_fcc8_sse4_1_impl<mem_access::unaligned>( dst, src );
    }
}
}

img_filter::transform_function_type     img_filter::transform::get_transform_fcc8_to_fcc16_sse41( const img::img_type& dst, const img::img_type& src )
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }
    if( can_convert_fcc8_to_fcc16( dst, src ) ) {
        return transform_fcc8_to_fcc16_sse4_1_v0;
    }
    return nullptr;
}

img_filter::transform_function_type     img_filter::transform::get_transform_fcc16_to_fcc8_sse41( const img::img_type& dst, const img::img_type& src )
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }
    if( can_convert_fcc16_to_fcc8( dst, src ) ) {
        return transform_fcc16_to_fcc8_sse4_1_v0;
    }
    return nullptr;
}

