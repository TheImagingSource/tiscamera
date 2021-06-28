
#include "wb_apply.h"

#include "../../simd_helper/use_simd_sse41.h"

namespace {

using namespace simd::sse;

FORCEINLINE
__m128i	    wb_by16_sse4_1_step_( __m128i src, __m128i mul ) noexcept
{
    const __m128i lo = _mm_cvtepu16_epi32( src );
    const __m128i hi = _mm_unpackhi_epi16( src, _mm_setzero_si128() );

    const __m128i tmp_lo = _mm_mullo_epi32( lo, mul );
    const __m128i tmp_hi = _mm_mullo_epi32( hi, mul );

    const __m128i res_lo = _mm_srli_epi32( tmp_lo, 6 );
    const __m128i res_hi = _mm_srli_epi32( tmp_hi, 6 );

    return _mm_packus_epi32( res_lo, res_hi );
}

static void    wb_by16_line_sse2_loop( uint16_t* line, int dim_x, __m128i f0 ) noexcept
{
    for( int x = 0; x < (dim_x - 7); x += 8 )
    {
        const __m128i src = load_si128u( line + x );

        const auto res = wb_by16_sse4_1_step_( src, f0 );

        store_u( line + x, res );
    }
}

static void    wb_by16_line_sse4_1( uint16_t* line, int dim_x, __m128i f0, __m128i f1 ) noexcept
{
    assert( dim_x >= 8 );

    if( dim_x % 8 == 0 )
    {
        wb_by16_line_sse2_loop( line, dim_x, f0 );
    }
    else
    {
        const __m128i last = load_si128u( line + dim_x - 8 );

        wb_by16_line_sse2_loop( line, dim_x, f0 );

        if( dim_x % 2 == 0 ) {
            store_u( line + dim_x - 8, wb_by16_sse4_1_step_( last, f0 ) );
        } else {
            store_u( line + dim_x - 8, wb_by16_sse4_1_step_( last, f1 ) );
        }
    }
}

static void	wb_by16_image_sse4_1( img::img_descriptor dst, __m128i factor00, __m128i factor01, __m128i factor10, const __m128i& factor11 ) noexcept
{
	int y = 0;
	for( ; y < (dst.dim.cy - 1); y += 2 )
	{
		uint16_t* dst_line0 = img::get_line_start<uint16_t>( dst, y + 0 );
		uint16_t* dst_line1 = img::get_line_start<uint16_t>( dst, y + 1 );

		wb_by16_line_sse4_1( dst_line0, dst.dim.cx, factor00, factor01 );
		wb_by16_line_sse4_1( dst_line1, dst.dim.cx, factor10, factor11 );
	}
	if( y == (dst.dim.cy - 1) )
	{
        uint16_t* dst_line0 = img::get_line_start<uint16_t>( dst, y + 0 );
        wb_by16_line_sse4_1( dst_line0, dst.dim.cx, factor00, factor01 );
    }
}

static __m128i fill_factors( uint8_t fac0, uint8_t fac1 ) noexcept
{
	return _mm_set_epi32( fac1, fac0, fac1, fac0 );
}

}

void		img_filter::whitebalance::detail::apply_wb_by16_sse4_1( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb )
{
    if( wb_r == 64 && wb_gr == 64 && wb_b == 64 && wb_gb == 64 ) {
        return;
    }
    assert( dst.dim.cx >= 8 && dst.dim.cy >= 1 );

	__m128i bg = fill_factors( wb_b, wb_gb );
	__m128i gb = fill_factors( wb_gb, wb_b );
	__m128i gr = fill_factors( wb_gr, wb_r );
	__m128i rg = fill_factors( wb_r, wb_gr );

	switch( dst.fourcc_type() )
	{
	case img::fourcc::BGGR16:	wb_by16_image_sse4_1( dst, bg, gb, gr, rg ); break;
	case img::fourcc::GBRG16:	wb_by16_image_sse4_1( dst, gb, bg, rg, gr ); break;
	case img::fourcc::GRBG16:	wb_by16_image_sse4_1( dst, gr, rg, bg, gb ); break;
	case img::fourcc::RGGB16:	wb_by16_image_sse4_1( dst, rg, gr, gb, bg ); break;
    default:
        break;
	};
}
