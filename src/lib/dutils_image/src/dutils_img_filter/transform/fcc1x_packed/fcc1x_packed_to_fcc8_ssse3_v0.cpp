

#include "fcc1x_packed_to_fcc.h"

#include "fcc1x_packed_to_fcc8_internal_loop.h"

#include "../../simd_helper/use_simd_ssse3.h"

using namespace fcc1x_packed_internal;

using namespace simd::sse;

namespace
{

void transform_fcc12_packed_to_fcc8_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( src.dim.cx % 2 == 0 );

    static const __m128i scatter_upper = INIT_M128i_REG( 0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x09, 0x0B, 0x0C, 0x0E, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 16); x += 8 )                                                   // we currently read in 16 bytes but only use 12 
        {
            // uint8_t* + 0    PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0
            // uint8_t* + 16   ________________PFPxPEPDPxPCPBPx
            __m128i v0 = load_si128u( src_line + (x / 2) * 3 );                                 // v0 =     PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0

            __m128i rvals = _mm_shuffle_epi8( v0, scatter_upper );                              // t0 =     __________PfPePcPbP9P8P6P5P3P2P0

            store_u( dst_line + x, rvals );
        }
        
        transform_fcc12_packed_to_fcc8_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc12_mipi_to_fcc8_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( src.dim.cx % 2 == 0 );

    static const __m128i scatter_upper = INIT_M128i_REG( 0x00, 0x01, 0x03, 0x04, 0x06, 0x07, 0x09, 0x0A, 0x0C, 0x0D, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 16); x += 8 )       // we currently read in 16 bytes but only write 8 
        {
            // uint8_t* + 0    PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0
            // uint8_t* + 16   ________________PFPxPEPDPxPCPBPx
            __m128i v0 = load_si128u( src_line + (x / 2) * 3 );                                 // v0 =     PAP9PxP8P7PxP6P5PxP4P3PxP2P1PxP0

            __m128i rvals = _mm_shuffle_epi8( v0, scatter_upper );                              // t0 =     __________PfPePcPbP9P8P6P5P3P2P0

            store_epi64u( dst_line + x, rvals );
        }

        transform_fcc12_mipi_to_fcc8_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc12_spacked_to_fcc8_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( src.dim.cx % 2 == 0 );

    // u8 11111111'11110000'00000000

    // scatter_mid, fetch all Px into uint16_t
    static const __m128i scatter_p0 = INIT_M128i_REG( 0, 1, 3, 4, 6, 7, 9, 10, 12, 13, 15, 0x80, 0x80, 0x80, 0x80, 0x80 );
    static const __m128i scatter_p1 = INIT_M128i_REG( 0x80, 2, 0x80, 5, 0x80, 8, 0x80, 11, 0x80, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 16); x += 8 )       // we currently read in 16 bytes but only use 8 
        {
            // v0 = FEDC'BA98'7654'3210
            // p0 =[21], p1 = [54]
            __m128i v0 = load_si128u( src_line + (x / 2) * 3 );

            __m128i tmp0 = _mm_shuffle_epi8( v0, scatter_p0 ); // 
            __m128i tmp2 = _mm_shuffle_epi8( v0, scatter_p1 );

            __m128i tmp3 = _mm_srli_epi16( tmp0, 4 );                           // u16=0FFF
            __m128i tmp4 = _mm_and_si128( tmp3, _mm_set1_epi16( 0x00FF ) );     // u16=00FF
            __m128i res = _mm_or_si128( tmp2, tmp4 );

            store_epi64u( dst_line + x, res );
        }

        transform_fcc12_spacked_to_fcc8_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc10_spacked_to_fcc8_ssse3_v0( img::img_descriptor dst, img::img_descriptor src )
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
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

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

            auto res2 = _mm_srli_epi16( res0, 8 );
			auto res3 = _mm_srli_epi16( res1, 8 );
            auto res4 = _mm_packus_epi16( res2, res3 );

            store_u( dst_line + x + 0, res4 );
        }
        transform_fcc10_spacked_to_fcc8_c_line( src_line + (x / 4) * 5, dst_line + x, src.dim.cx - x );
    }
}
}

auto img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_ssse3( const img::img_type& dst, const img::img_type& src ) -> transform_function_type
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }

    using namespace img::fcc1x_packed;

    if( !is_accepted_dst_fcc8( dst.fourcc_type() ) ) {
        return nullptr;
    }

    switch( get_fcc1x_pack_type( src.fourcc_type() ) )
    {
    case fccXX_pack_type::fcc12:                return nullptr; // #TODO implement transform_fcc12_to_fcc8_ssse3_v0
    case fccXX_pack_type::fcc12_packed:         return ::transform_fcc12_packed_to_fcc8_ssse3_v0;
    case fccXX_pack_type::fcc12_mipi:           return ::transform_fcc12_mipi_to_fcc8_ssse3_v0;
    case fccXX_pack_type::fcc12_spacked:        return ::transform_fcc12_spacked_to_fcc8_ssse3_v0;

    case fccXX_pack_type::fcc10:                return nullptr; // #TODO implement transform_fcc10_to_fcc8_ssse3_v0
    case fccXX_pack_type::fcc10_spacked:        return ::transform_fcc10_spacked_to_fcc8_ssse3_v0;
    case fccXX_pack_type::fcc10_mipi:           return nullptr; // #TODO implement transform_fcc10_mipi_to_fcc8_ssse3_v0

    case fccXX_pack_type::invalid:              return nullptr;
    };

    return nullptr;
}