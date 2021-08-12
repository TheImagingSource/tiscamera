

#include "fcc1x_packed_to_fcc.h"

#include "../../simd_helper/use_simd_ssse3.h"

#include "fcc1x_packed_to_fcc16_internal.h"
#include "fcc1x_packed_to_fcc16_internal_loop.h"

using namespace fcc1x_packed_internal;

using namespace simd::sse;

namespace
{
    FORCEINLINE void transform_fcc12_to_fcc16_step( int x, uint16_t* dst_line, const uint16_t* src_line )
    {
        __m128i v0 = load_si128u( src_line + x );
        __m128i v0_tmp = _mm_slli_epi16( v0, 4 );
        store_u( dst_line + x, v0_tmp );
    }

void transform_fcc12_to_fcc16_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto src_line = img::get_line_start<const uint16_t>( src, y );
        auto dst_line = img::get_line_start<uint16_t>( dst, y );

        for( int x = 0; x < (src.dim.cx - 8); x += 8 )
        {
            transform_fcc12_to_fcc16_step( x, dst_line, src_line );
        }
        transform_fcc12_to_fcc16_step( src.dim.cx - 8, dst_line, src_line );
    }
}

FORCEINLINE void transform_fcc10_to_fcc16_step( int x, uint16_t* dst_line, const uint16_t* src_line )
{
    __m128i v0 = load_si128u( src_line + x );
    __m128i v0_tmp = _mm_slli_epi16( v0, 6 );
    store_u( dst_line + x, v0_tmp );
}

void transform_fcc10_to_fcc16_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto src_line = img::get_line_start<const uint16_t>( src, y );
        auto dst_line = img::get_line_start<uint16_t>( dst, y );

        for( int x = 0; x < (src.dim.cx - 8); x += 8 )
        {
            transform_fcc10_to_fcc16_step( x, dst_line, src_line );
        }
        transform_fcc10_to_fcc16_step( src.dim.cx - 8, dst_line, src_line );
    }
}

void transform_fcc12_packed_to_fcc16_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( src.dim.cx % 2 == 0 );

    static const __m128i scatter_upper = INIT_M128i_REG( 0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0B, 0x0C, 0x0E, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_mid = INIT_M128i_REG( 0x01, 0x80, 0x04, 0x80, 0x07, 0x80, 0x0A, 0x80, 0x0D, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto src_line = img::get_line_start<uint8_t>( src, y );
        auto dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 12); x += 8 )                                                   // we currently read in 16 bytes but only use 12 
        {
            // uint8_t* + 0    PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0
            // uint8_t* + 16   ________________PFPxPEPDPxPCPBPx
            __m128i v0 = load_si128u( src_line + (x / 2) * 3 );                                 // v0 =     PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0

            __m128i tmp_upper = _mm_shuffle_epi8( v0, scatter_upper );
            __m128i tmp_lower = _mm_shuffle_epi8( v0, scatter_mid );                            // u16      ________________Px76Px54Px32Px10
            __m128i tmp_lower_1357 = _mm_slli_epi16( _mm_srli_epi16( tmp_lower, 4 ), 4 );       // u16      ________________Px_7Px_5Px_3Px_1
            __m128i tmp_lower_0246 = _mm_srli_epi16( _mm_slli_epi16( tmp_lower, 12 ), 8 );      // u16      ________________Px_6Px_4Px_2Px_0

            __m128i tmp_lower_2 = _mm_unpacklo_epi16( tmp_lower_0246, tmp_lower_1357 );         // u16      Px_7Px_6Px_5Px_4Px_3Px_2Px_1Px_0
            __m128i tmp_lower_3 = _mm_packus_epi16( tmp_lower_2, _mm_setzero_si128() );

            __m128i rvals = _mm_unpacklo_epi8( tmp_lower_3, tmp_upper );

            store_u( dst_line + x, rvals );
        }
        
        transform_fcc12_packed_to_fcc16_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc12_mipi_to_fcc16_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( src.dim.cx % 2 == 0 );

    static const __m128i scatter_upper = INIT_M128i_REG( 0x00, 0x01, 0x03, 0x04, 0x06, 0x07, 0x09, 0x0A, 0x0C, 0x0D, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_mid = INIT_M128i_REG( 0x02, 0x80, 0x05, 0x80, 0x08, 0x80, 0x0B, 0x80, 0x0E, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto src_line = img::get_line_start<uint8_t>( src, y );
        auto dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 12); x += 8 )       // we currently read in 16 bytes but only use 8 
        {
            // uint8_t* + 0    PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0
            // uint8_t* + 16   ________________PFPxPEPDPxPCPBPx
            __m128i v0 = load_si128u( src_line + (x / 2) * 3 );                                 // v0 =     PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0

            __m128i tmp_upper = _mm_shuffle_epi8( v0, scatter_upper );
            __m128i tmp_lower = _mm_shuffle_epi8( v0, scatter_mid );                            // u16      ________________Px76Px54Px32Px10
            __m128i tmp_lower_1357 = _mm_slli_epi16( _mm_srli_epi16( tmp_lower, 4 ), 4 );       // u16      ________________Px_7Px_5Px_3Px_1
            __m128i tmp_lower_0246 = _mm_srli_epi16( _mm_slli_epi16( tmp_lower, 12 ), 8 );      // u16      ________________Px_6Px_4Px_2Px_0

            __m128i tmp_lower_2 = _mm_unpacklo_epi16( tmp_lower_0246, tmp_lower_1357 );         // u16      Px_7Px_6Px_5Px_4Px_3Px_2Px_1Px_0
            __m128i tmp_lower_3 = _mm_packus_epi16( tmp_lower_2, _mm_setzero_si128() );

            __m128i rvals = _mm_unpacklo_epi8( tmp_lower_3, tmp_upper );

            store_u( dst_line + x, rvals );
        }

        transform_fcc12_mipi_to_fcc16_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}


void transform_fcc12_spacked_to_fcc16_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( src.dim.cx % 2 == 0 );

    // u8 11111111'11110000'00000000

    //val |= ((uint32_t( src_ptr[0] & 0xFF ) << 4) | (uint32_t( src_ptr[1] & 0x0F ) << 12)) << 0;
    //val |= ((uint32_t( src_ptr[1] & 0xF0 ) << 0) | (uint32_t( src_ptr[2] & 0xFF ) << 8)) << 16;

    // scatter_mid, fetch all Px into uint16_t
    static const __m128i scatter_low = INIT_M128i_REG( 0, 0x80, 3, 0x80, 6, 0x80, 9, 0x80, 12, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_mid = INIT_M128i_REG( 1, 0x80, 4, 0x80, 7, 0x80, 10, 0x80, 13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_hig = INIT_M128i_REG( 0x80, 2, 0x80, 5, 0x80, 8, 0x80, 11, 0x80, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint16_t* dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 12); x += 8 )       // we currently read in 16 bytes but only use 8 
        {
            // uint8_t* + 0    PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0
            // uint8_t* + 16   ________________PFPxPEPDPxPCPBPx
            __m128i v0 = load_si128u( src_line + (x / 2) * 3 );                                 // v0 =     PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0

            __m128i tmp0 = _mm_shuffle_epi8( v0, scatter_low );
            __m128i tmp1 = _mm_shuffle_epi8( v0, scatter_mid );// u16 = 0bXXXXXXXX'11110000
            __m128i tmp2 = _mm_shuffle_epi8( v0, scatter_hig );

            __m128i tmp3 = _mm_slli_epi16( tmp0, 4 );                           // u16=0b____0000'00000000
            __m128i tmp4 = _mm_slli_epi16( tmp1, 12 );                          // u16=0b0000____'________
            __m128i tmp5 = _mm_slli_epi16( _mm_srli_epi16( tmp1, 4 ), 4  );     // u16=0b________'1111____

            __m128i tmp10 = _mm_or_si128( tmp3, tmp4 );         // u16[i] = __, p6, p4, p2, p0
            __m128i tmp11 = _mm_or_si128( tmp2, tmp5 );         // u16[i] = __, p7, p5, p3, p1

            __m128i res = _mm_unpacklo_epi16( tmp10, tmp11 );   // u16[i] = p7, ... , p2, p1, p0

            store_u( dst_line + x, res );
        }

        transform_fcc12_spacked_to_fcc16_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc10_spacked_to_fcc16_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( src.dim.cx % 4 == 0 );

    // src = u8[] = XXc2c2c2c2c2c1c1'c1c1c1c0c0c0c0c0       (c denotes cluster)

    //  clutter, bits = 33333333'33222222'22221111'11111100'00000000

    // these look very thinly stuffed, maybe better arrangements could be found for better register usage
    static const __m128i scatter_0 = INIT_M128i_REG( 0, 1, 5, 6, 10, 11, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_1 = INIT_M128i_REG( 1, 2, 6, 7, 11, 12, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_2 = INIT_M128i_REG( 2, 3, 7, 8, 12, 13, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_3 = INIT_M128i_REG( 3, 4, 8, 9, 13, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint16_t* dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 16); x += 12 )
        {
            __m128i v0 = load_si128u( src_line + (x / 4) * 5 );

            __m128i tmp0 = _mm_shuffle_epi8( v0, scatter_0 );
            __m128i tmp1 = _mm_shuffle_epi8( v0, scatter_1 );
            __m128i tmp2 = _mm_shuffle_epi8( v0, scatter_2 );
            __m128i tmp3 = _mm_shuffle_epi8( v0, scatter_3 );

            // these are not the 'real' u16 values, because we have to shift these into the upper 10 bits of the u16 register, this is done later to 
            // reduce the count of shifts
            __m128i tmp4 = tmp0;                           // u16[] = p10, p05, p00
            __m128i tmp5 = _mm_srli_epi16( tmp1, 2 );      // u16[] = p11, p06, p01
            __m128i tmp6 = _mm_srli_epi16( tmp2, 4 );      // u16[] = p12, p07, p02
            __m128i tmp7 = _mm_srli_epi16( tmp3, 6 );      // 

            __m128i res_lo = _mm_unpacklo_epi16( tmp4, tmp5 );                  // u16[] = __, __, p11, p10, p06, p05, p01, p00
            __m128i res_hi = _mm_unpacklo_epi16( tmp6, tmp7 );
            __m128i res0 = _mm_unpacklo_epi32( res_lo, res_hi );
            __m128i res1 = _mm_unpackhi_epi32( res_lo, res_hi );

            res0 = _mm_slli_epi16( res0, 6 );
            res1 = _mm_slli_epi16( res1, 6 );

            // #TODO we could optimize this further by doing 24 at once, and then writing the result in 3 writes, instead of the 1 full and 1 half we are currently doing for 12

            store_u( dst_line + x + 0, res0 );
            store_epi64u( dst_line + x + 8, res1 );
        }
        transform_fcc10_spacked_to_fcc16_c_line( src_line + (x / 4) * 5, dst_line + x, src.dim.cx - x );
    }
}
}

auto img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_ssse3( const img::img_type& dst, const img::img_type& src ) -> transform_function_type
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }
    if( src.dim.cx < 8 ) {
        return nullptr;
    }

    using namespace img::fcc1x_packed;

    if( !is_accepted_dst_fcc16( dst.fourcc_type() ) ) {
        return nullptr;
    }

    switch( get_fcc1x_pack_type( src.fourcc_type() ) )
    {
    case fccXX_pack_type::fcc12:            return ::transform_fcc12_to_fcc16_ssse3_v0;
    case fccXX_pack_type::fcc12_packed:     return ::transform_fcc12_packed_to_fcc16_ssse3_v0;
    case fccXX_pack_type::fcc12_mipi:       return ::transform_fcc12_mipi_to_fcc16_ssse3_v0;
    case fccXX_pack_type::fcc12_spacked:    return ::transform_fcc12_spacked_to_fcc16_ssse3_v0;

    case fccXX_pack_type::fcc10:            return ::transform_fcc10_to_fcc16_ssse3_v0;
    case fccXX_pack_type::fcc10_spacked:    return ::transform_fcc10_spacked_to_fcc16_ssse3_v0;
    case fccXX_pack_type::fcc10_mipi:       return nullptr; // #TODO implement transform_fcc10_mipi_to_dst_ssse3

    case fccXX_pack_type::invalid:          return nullptr;
    };
    return nullptr;
}