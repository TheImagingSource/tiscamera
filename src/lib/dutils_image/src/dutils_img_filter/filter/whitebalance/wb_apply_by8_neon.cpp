

#include "wb_apply.h"

#include "../../simd_helper/use_simd_A64.h"

namespace {
        
FORCEINLINE
uint8x16_t	    wb_by8_step_( uint8x16_t src, uint16x8_t mul )
{
    uint16x8_t lo = vmovl_u8( vget_low_u8( src ) );
    uint16x8_t hi = vmovl_high_u8( src );

    uint8x8_t res_lo = vqshrn_n_u16( lo * mul, 6 );
    uint8x8_t res_hi = vqshrn_n_u16( hi * mul, 6 );

    return vcombine_u8( res_lo, res_hi );
}

FORCEINLINE
void    wb_by8_line_neon_loop( uint8_t* line, int dim_x, uint16x8_t f0 )
{
    for( int x = 0; x <= (dim_x - 16); x += 16 )
    {
        const uint8x16_t src = vld1q_u8( line + x );

        uint8x16_t res = wb_by8_step_( src, f0 );

        vst1q_u8( line + x, res );
    }
}

void    wb_by8_line_neon( uint8_t* line, int dim_x, uint16x8_t f0, uint16x8_t f1 )
{
    assert( dim_x >= 16 );

    if( dim_x % 16 == 0 )
    {
        wb_by8_line_neon_loop( line, dim_x, f0 );
    }
    else
    {
        const uint8x16_t last16 = vld1q_u8( line + dim_x - 16 );

        wb_by8_line_neon_loop( line, dim_x, f0 );

        if( dim_x % 2 == 0 ) {
            vst1q_u8( line + dim_x - 16, wb_by8_step_( last16, f0 ) );
        } else {
            vst1q_u8( line + dim_x - 16, wb_by8_step_( last16, f1 ) );
        }
    }
}

void	wb_by8_image_( img::img_descriptor dst, uint16x8_t factor00, uint16x8_t factor01, uint16x8_t factor10, const uint16x8_t& factor11 )
{
    int y = 0;
    for( ; y < (dst.dim.cy - 1); y += 2 )
    {
        uint8_t* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );
        uint8_t* dst_line1 = img::get_line_start<uint8_t>( dst, y + 1 );

        wb_by8_line_neon( dst_line0, dst.dim.cx, factor00, factor01 );
        wb_by8_line_neon( dst_line1, dst.dim.cx, factor10, factor11 );
    }
    if( y == (dst.dim.cy - 1) )
    {
        uint8_t* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );
        wb_by8_line_neon( dst_line0, dst.dim.cx, factor00, factor01 );
    }
}

}

void		img_filter::whitebalance::detail::apply_wb_by8_neon( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb )
{
    if( wb_r == 64 && wb_gr == 64 && wb_b == 64 && wb_gb == 64 ) {
        return;
    }
    assert( dst.dim.cx >= 16 );  // && dst.dim_y > 0

    const uint16x8_t bg = uint16x8_t{ wb_b, wb_gb, wb_b, wb_gb, wb_b, wb_gb, wb_b, wb_gb };
    const uint16x8_t gb = uint16x8_t{ wb_gb, wb_b, wb_gb, wb_b, wb_gb, wb_b, wb_gb, wb_b };
    const uint16x8_t gr = uint16x8_t{ wb_gr, wb_r, wb_gr, wb_r, wb_gr, wb_r, wb_gr, wb_r };
    const uint16x8_t rg = uint16x8_t{ wb_r, wb_gr, wb_r, wb_gr, wb_r, wb_gr, wb_r, wb_gr };

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGGR8:	wb_by8_image_( dst, bg, gb, gr, rg ); break;
    case img::fourcc::GBRG8:	wb_by8_image_( dst, gb, bg, rg, gr ); break;
    case img::fourcc::GRBG8:	wb_by8_image_( dst, gr, rg, bg, gb ); break;
    case img::fourcc::RGGB8:	wb_by8_image_( dst, rg, gr, gb, bg ); break;
    default:
        break;

    };
}
