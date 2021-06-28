
#include "wb_apply.h"

#include "../../simd_helper/use_simd_A64.h"

#include <cassert>

namespace
{

FORCEINLINE
uint16x8_t	    wb_by16_neon_step_( uint16x8_t src, uint16x8_t mul ) noexcept
{
    const uint32x4_t tmp_lo = vmull_u16( vget_low_u16( src ), vget_low_u16( mul ) );
    const uint32x4_t tmp_hi = vmull_high_u16( src, mul );

    const uint16x4_t res_lo = vqshrn_n_u32( tmp_lo, 6 );
    const uint16x4_t res_hi = vqshrn_n_u32( tmp_hi, 6 );

    return vcombine_u16( res_lo, res_hi );
}

FORCEINLINE
static void    wb_by16_line_neon_loop( uint16_t* line, int dim_x, uint16x8_t f0 ) noexcept
{
    for( int x = 0; x < (dim_x - 7); x += 8 )
    {
        const uint16x8_t res = wb_by16_neon_step_( vld1q_u16( line + x ), f0 );

        vst1q_u16( line + x, res );
    }
}

FORCEINLINE
static void    wb_by16_line_neon( uint16_t* line, int dim_x, uint16x8_t f0, uint16x8_t f1 ) noexcept
{
    assert( dim_x >= 8 );

    if( dim_x % 8 == 0 )
    {
        wb_by16_line_neon_loop( line, dim_x, f0 );
    }
    else
    {
        const uint16x8_t last = vld1q_u16( line + dim_x - 8 );

        wb_by16_line_neon_loop( line, dim_x, f0 );

        if( dim_x % 2 == 0 ) {
            vst1q_u16( line + dim_x - 8, wb_by16_neon_step_( last, f0 ) );
        }else {
            vst1q_u16( line + dim_x - 8, wb_by16_neon_step_( last, f1 ) );
        }
    }
}

static void	wb_by16_image_neon( img::img_descriptor dst, uint16x8_t factor00, uint16x8_t factor01, uint16x8_t factor10, uint16x8_t factor11 ) noexcept
{
	int y = 0;
	for( ; y < (dst.dim.cy - 1); y += 2 )
	{
		uint16_t* dst_line0 = img::get_line_start<uint16_t>( dst, y + 0 );
		uint16_t* dst_line1 = img::get_line_start<uint16_t>( dst, y + 1 );

		wb_by16_line_neon( dst_line0, dst.dim.cx, factor00, factor01 );
		wb_by16_line_neon( dst_line1, dst.dim.cx, factor10, factor11 );
	}
	if( y == (dst.dim.cy - 1) )
	{
        uint16_t* dst_line0 = img::get_line_start<uint16_t>( dst, y + 0 );
        wb_by16_line_neon( dst_line0, dst.dim.cx, factor00, factor01 );
    }
}


}

void		img_filter::whitebalance::detail::apply_wb_by16_neon( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb )
{
    if( wb_r == 64 && wb_gr == 64 && wb_b == 64 && wb_gb == 64 ) {
        return;
    }
    assert( dst.dim.cx >= 8 );  // && dst.dim_y > 0

    const uint16x8_t bg = uint16x8_t{ wb_b, wb_gb, wb_b, wb_gb, wb_b, wb_gb, wb_b, wb_gb };
    const uint16x8_t gb = uint16x8_t{ wb_gb, wb_b, wb_gb, wb_b, wb_gb, wb_b, wb_gb, wb_b };
    const uint16x8_t gr = uint16x8_t{ wb_gr, wb_r, wb_gr, wb_r, wb_gr, wb_r, wb_gr, wb_r };
    const uint16x8_t rg = uint16x8_t{ wb_r, wb_gr, wb_r, wb_gr, wb_r, wb_gr, wb_r, wb_gr };

	switch( dst.fourcc_type() )
	{
    case img::fourcc::BGGR16:	wb_by16_image_neon( dst, bg, gb, gr, rg ); break;
    case img::fourcc::GBRG16:	wb_by16_image_neon( dst, gb, bg, rg, gr ); break;
    case img::fourcc::GRBG16:	wb_by16_image_neon( dst, gr, rg, bg, gb ); break;
    case img::fourcc::RGGB16:	wb_by16_image_neon( dst, rg, gr, gb, bg ); break;
    default:
        break;
	};
}
