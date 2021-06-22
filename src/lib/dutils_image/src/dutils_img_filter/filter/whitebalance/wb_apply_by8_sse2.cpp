

#include "wb_apply.h"

#include "../../simd_helper/use_simd_ssse3.h"

namespace {

using namespace simd::sse;
    
FORCEINLINE
__m128i	    wb_by8_sse2_step_( __m128i src, __m128i mul )
{
    __m128i lo = _mm_unpacklo_epi8( src, _mm_setzero_si128() );
    __m128i hi = _mm_unpackhi_epi8( src, _mm_setzero_si128() );

    __m128i tmp_lo = _mm_mullo_epi16( lo, mul );
    __m128i tmp_hi = _mm_mullo_epi16( hi, mul );

    __m128i res_lo = _mm_srli_epi16( tmp_lo, 6 );
    __m128i res_hi = _mm_srli_epi16( tmp_hi, 6 );

    return _mm_packus_epi16( res_lo, res_hi );
}

static void    wb_by8_line_sse2_loop( uint8_t* line, int dim_x, __m128i f0 )
{
    for( int x = 0; x < (dim_x - 15); x += 16 )
    {
        const __m128i src = load_si128u( line + x );

        auto res = wb_by8_sse2_step_( src, f0 );

        store_u( line + x, res );
    }
}

static void    wb_by8_line_sse2( uint8_t* line, int dim_x, __m128i f0, __m128i f1 )
{
    assert( dim_x >= 16 );

    if( dim_x % 16 == 0 )
    {
        wb_by8_line_sse2_loop( line, dim_x, f0 );
    }
    else
    {
        const __m128i last16 = load_si128u( line + dim_x - 16 );

        wb_by8_line_sse2_loop( line, dim_x, f0 );

        if( dim_x % 2 == 0 ) {
            store_u( line + dim_x - 16, wb_by8_sse2_step_( last16, f0 ) );
        } else {
            store_u( line + dim_x - 16, wb_by8_sse2_step_( last16, f1 ) );
        }
    }
}

void	wb_by8_image_sse2( img::img_descriptor dst, __m128i factor00, __m128i factor01, __m128i factor10, __m128i factor11 )
{
    int y = 0;
    for( ; y < (dst.dim.cy - 1); y += 2 )
    {
        uint8_t* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );
        uint8_t* dst_line1 = img::get_line_start<uint8_t>( dst, y + 1 );

        wb_by8_line_sse2( dst_line0, dst.dim.cx, factor00, factor01 );
        wb_by8_line_sse2( dst_line1, dst.dim.cx, factor10, factor11 );
    }
    if( y == (dst.dim.cy - 1) )
    {
        uint8_t* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );

        wb_by8_line_sse2( dst_line0, dst.dim.cx, factor00, factor01 );
    }
}

static __m128i fill_factors( uint8_t fac0, uint8_t fac1 )
{
    return _mm_set_epi16( fac1, fac0, fac1, fac0, fac1, fac0, fac1, fac0 );
}

}

void		img_filter::whitebalance::detail::apply_wb_by8_sse2( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb )
{
    if( wb_r == 64 && wb_gr == 64 && wb_b == 64 && wb_gb == 64 ) {
        return;
    }

    assert( dst.dim.cx >= 16 && dst.dim.cy >= 1 );

    const __m128i bg = fill_factors( wb_b, wb_gb );
    const __m128i gb = fill_factors( wb_gb, wb_b );
    const __m128i gr = fill_factors( wb_gr, wb_r );
    const __m128i rg = fill_factors( wb_r, wb_gr );

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGGR8:	wb_by8_image_sse2( dst, bg, gb, gr, rg ); break;
    case img::fourcc::GBRG8:	wb_by8_image_sse2( dst, gb, bg, rg, gr ); break;
    case img::fourcc::GRBG8:	wb_by8_image_sse2( dst, gr, rg, bg, gb ); break;
    case img::fourcc::RGGB8:	wb_by8_image_sse2( dst, rg, gr, gb, bg ); break;
    default:
        break;
    };
}
