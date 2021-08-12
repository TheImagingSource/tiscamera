
#include "transform_mono_to_bgr.h"
#include "transform_mono_to_bgr_internal.h"

#include "../../simd_helper/use_simd_sse41.h"


namespace
{
    
using namespace transform_mono_to_bgr_internal;

using namespace img::pixel_type;
using namespace simd::sse;

FORCEINLINE
void	convert_MONO8_to_BGR24_store_sse41( uint8_t* dst_ptr, __m128i y8 )
{
    static const __m128i mask0 = INIT_M128i_REG( 0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x4, 0x4, 0x4, 0x5 );
    static const __m128i mask1 = INIT_M128i_REG( 0x5, 0x5, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x8, 0x8, 0x8, 0x9, 0x9, 0x9, 0xA, 0xA );
    static const __m128i mask2 = INIT_M128i_REG( 0xA, 0xB, 0xB, 0xB, 0xC, 0xC, 0xC, 0xD, 0xD, 0xD, 0xE, 0xE, 0xE, 0xF, 0xF, 0xF );

    auto r0 = _mm_shuffle_epi8( y8, mask0 );
    auto r1 = _mm_shuffle_epi8( y8, mask1 );
    auto r2 = _mm_shuffle_epi8( y8, mask2 );

    store_u( dst_ptr +  0, r0 );
    store_u( dst_ptr + 16, r1 );
    store_u( dst_ptr + 32, r2 );
}

void	transform_MONO8_to_BGR24_sse41( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGR24>( dst, y );
        int x = 0;
        for( ; x < (dst.dim.cx - 15); x += 16 )
        {
            __m128i y8_vals = load_si128u( src_line + x );
            convert_MONO8_to_BGR24_store_sse41( reinterpret_cast<uint8_t*>( dst_line + x ), y8_vals );
        }
        transform_MONO8_to_BGR24_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}


FORCEINLINE
void	convert_MONO8_to_BGRA32_store_sse41( BGRA32* dst_ptr, __m128i y8 )
{
    static const __m128i mask0 = INIT_M128i_REG( 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x1, 0x2, 0x2, 0x2, 0x2, 0x3, 0x3, 0x3, 0x3 );
    static const __m128i mask1 = INIT_M128i_REG( 0x4, 0x4, 0x4, 0x4, 0x5, 0x5, 0x5, 0x5, 0x6, 0x6, 0x6, 0x6, 0x7, 0x7, 0x7, 0x7 );
    static const __m128i mask2 = INIT_M128i_REG( 0x8, 0x8, 0x8, 0x8, 0x9, 0x9, 0x9, 0x9, 0xA, 0xA, 0xA, 0xA, 0xB, 0xB, 0xB, 0xB );
    static const __m128i mask3 = INIT_M128i_REG( 0xC, 0xC, 0xC, 0xC, 0xD, 0xD, 0xD, 0xD, 0xE, 0xE, 0xE, 0xE, 0xF, 0xF, 0xF, 0xF );
    
    static const __m128i mask_000000FF = INIT_M128i_REG( 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF );

    auto r0 = _mm_shuffle_epi8( y8, mask0 );
    auto r1 = _mm_shuffle_epi8( y8, mask1 );
    auto r2 = _mm_shuffle_epi8( y8, mask2 );
    auto r3 = _mm_shuffle_epi8( y8, mask3 );

    store_u( dst_ptr +  0, _mm_or_si128( r0, mask_000000FF ) );
    store_u( dst_ptr +  4, _mm_or_si128( r1, mask_000000FF ) );
    store_u( dst_ptr +  8, _mm_or_si128( r2, mask_000000FF ) );
    store_u( dst_ptr + 12, _mm_or_si128( r3, mask_000000FF ) );
}

void	transform_MONO8_to_BGRA32_sse41( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA32>( dst, y );
        
        int x = 0;
        for( ; x < (dst.dim.cx - 15); x += 16 )
        {
            __m128i y8_vals = load_si128u( src_line + x );
            convert_MONO8_to_BGRA32_store_sse41( dst_line + x, y8_vals );
        }
        transform_MONO8_to_BGRA32_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}

FORCEINLINE
void	convert_MONO8_to_BGRA64_store_sse41( BGRA64* dst_ptr, __m128i y8 )
{
    static const __m128i mask0 =    INIT_M128i_REG( 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x01, 0xFF, 0x01, 0xFF, 0x01, 0xFF, 0xFF );
    static const __m128i mask1 =    INIT_M128i_REG( 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0x02, 0xFF, 0xFF, 0xFF, 0x03, 0xFF, 0x03, 0xFF, 0x03, 0xFF, 0xFF );
    static const __m128i mask2 =    INIT_M128i_REG( 0xFF, 0x04, 0xFF, 0x04, 0xFF, 0x04, 0xFF, 0xFF, 0xFF, 0x05, 0xFF, 0x05, 0xFF, 0x05, 0xFF, 0xFF );
    static const __m128i mask3 =    INIT_M128i_REG( 0xFF, 0x06, 0xFF, 0x06, 0xFF, 0x06, 0xFF, 0xFF, 0xFF, 0x07, 0xFF, 0x07, 0xFF, 0x07, 0xFF, 0xFF );

    static const __m128i mask0FF =  INIT_M128i_REG( 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF );

    auto r0 = _mm_shuffle_epi8( y8, mask0 );
    auto r1 = _mm_shuffle_epi8( y8, mask1 );
    auto r2 = _mm_shuffle_epi8( y8, mask2 );
    auto r3 = _mm_shuffle_epi8( y8, mask3 );

    store_u( dst_ptr + 0, _mm_or_si128( r0, mask0FF ) );
    store_u( dst_ptr + 2, _mm_or_si128( r1, mask0FF ) );
    store_u( dst_ptr + 4, _mm_or_si128( r2, mask0FF ) );
    store_u( dst_ptr + 6, _mm_or_si128( r3, mask0FF ) );
}

void	transform_MONO8_to_BGRA64_sse41( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA64>( dst, y );

        int x = 0;
        for( x = 0; x < (dst.dim.cx - 7); x += 8 )
        {
            __m128i y8_vals = load_epi64u( src_line + x );
            convert_MONO8_to_BGRA64_store_sse41( dst_line + x, y8_vals );
        }
        transform_MONO8_to_BGRA64_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}

FORCEINLINE
void	convert_MONO16_to_BGRA32_store_sse41( BGRA32* dst_ptr, __m128i y16 )
{
    static const __m128i mask0 = INIT_M128i_REG( 0x1, 0x1, 0x1, 0x1, 0x3, 0x3, 0x3, 0x3, 0x5, 0x5, 0x5, 0x5, 0x7, 0x7, 0x7, 0x7 );
    static const __m128i mask1 = INIT_M128i_REG( 0x9, 0x9, 0x9, 0x9, 0xB, 0xB, 0xB, 0xB, 0xD, 0xD, 0xD, 0xD, 0xF, 0xF, 0xF, 0xF );

    static const __m128i mask_000000FF = INIT_M128i_REG( 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0xFF );

    auto r0 = _mm_shuffle_epi8( y16, mask0 );
    auto r1 = _mm_shuffle_epi8( y16, mask1 );

    store_u( dst_ptr + 0, _mm_or_si128( r0, mask_000000FF ) );
    store_u( dst_ptr + 4, _mm_or_si128( r1, mask_000000FF ) );
}

void	transform_MONO16_to_BGRA32_sse41( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint16_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA32>( dst, y );

        int x = 0;
        for( ; x < (dst.dim.cx - 7); x += 8 )
        {
            __m128i y8_vals = load_si128u( src_line + x );
            convert_MONO16_to_BGRA32_store_sse41( dst_line + x, y8_vals );
        }
        transform_MONO16_to_BGRA32_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}

//FORCEINLINE
void	convert_MONO16_to_BGRA64_store_sse41( BGRA64* dst_ptr, __m128i y16 )
{
    static const __m128i mask0 = INIT_M128i_REG( 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0xFF, 0xFF, 0x02, 0x03, 0x02, 0x03, 0x02, 0x03, 0xFF, 0xFF );
    static const __m128i mask1 = INIT_M128i_REG( 0x04, 0x05, 0x04, 0x05, 0x04, 0x05, 0xFF, 0xFF, 0x06, 0x07, 0x06, 0x07, 0x06, 0x07, 0xFF, 0xFF );
    static const __m128i mask2 = INIT_M128i_REG( 0x08, 0x09, 0x08, 0x09, 0x08, 0x09, 0xFF, 0xFF, 0x0A, 0x0B, 0x0A, 0x0B, 0x0A, 0x0B, 0xFF, 0xFF );
    static const __m128i mask3 = INIT_M128i_REG( 0x0C, 0x0D, 0x0C, 0x0D, 0x0C, 0x0D, 0xFF, 0xFF, 0x0E, 0x0F, 0x0E, 0x0F, 0x0E, 0x0F, 0xFF, 0xFF );

    static const __m128i mask0FF = INIT_M128i_REG( 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF );

    auto r0 = _mm_shuffle_epi8( y16, mask0 );
    auto r1 = _mm_shuffle_epi8( y16, mask1 );
    auto r2 = _mm_shuffle_epi8( y16, mask2 );
    auto r3 = _mm_shuffle_epi8( y16, mask3 );

    store_u( dst_ptr + 0, _mm_or_si128( r0, mask0FF ) );
    store_u( dst_ptr + 2, _mm_or_si128( r1, mask0FF ) );
    store_u( dst_ptr + 4, _mm_or_si128( r2, mask0FF ) );
    store_u( dst_ptr + 6, _mm_or_si128( r3, mask0FF ) );
}

void	transform_MONO16_to_BGRA64_sse41( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA64>( dst, y );

        int x = 0;
        for( x = 0; x < (dst.dim.cx - 7); x += 8 )
        {
            __m128i y8_vals = load_si128u( src_line + x );
            convert_MONO16_to_BGRA64_store_sse41( dst_line + x, y8_vals );
        }
        transform_MONO16_to_BGRA64_c_line( dst.dim.cx - x, src_line + x, dst_line + x );
    }
}

}

img_filter::transform_function_type     img_filter::transform::get_transform_mono_to_bgr_sse41( const img::img_type& dst, const img::img_type& src )
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }
    if( src.fourcc_type() == img::fourcc::MONO8 )
    {
        switch( dst.fourcc_type() )
        {
        case img::fourcc::BGR24:        return transform_MONO8_to_BGR24_sse41;
        case img::fourcc::BGRA32:       return transform_MONO8_to_BGRA32_sse41;
        case img::fourcc::BGRA64:       return transform_MONO8_to_BGRA64_sse41;
        default:
            return nullptr;
        }
    }
    if( src.fourcc_type() == img::fourcc::MONO16 )
    {
        switch( dst.fourcc_type() )
        {
        //case img::fourcc::BGR24:        return transform_MONO8_to_BGR24_sse41;
        case img::fourcc::BGRA32:       return transform_MONO16_to_BGRA32_sse41;
        case img::fourcc::BGRA64:       return transform_MONO16_to_BGRA64_sse41;
        default:
            return nullptr;
        }
    }
    return nullptr;
}
