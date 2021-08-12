
#include "wb_apply.h"

namespace {

FORCEINLINE int wb_pixel_c_16bit( int pixel_value, int fac )
{
    int val = (pixel_value * fac) / 64;
    return  val > 0xFFFF ? 0xFFFF : val;
}

static void wb_line_c_16bit( uint16_t* dest_line, uint16_t* src_line, int dim_x, int mul0, int mul1 )
{
    for( int x = 0; x < (dim_x - 1); x += 2 )
    {
        int v0 = wb_pixel_c_16bit( src_line[x + 0], mul0 );
        int v1 = wb_pixel_c_16bit( src_line[x + 1], mul1 );
        *((uint32_t*)(dest_line + x)) = (uint32_t)(v1 << 16 | v0);
    }
    if( dim_x % 2 == 1 )
    {
        dest_line[dim_x-1] = (uint16_t) wb_pixel_c_16bit( src_line[dim_x-1], mul0 );
    }
}

static void	wb_image_c_16bit( img::img_descriptor dst, int mul_even_0, int mul_even_1, int mul_odd_0, int mul_odd_1 )
{
    int dim_x = dst.dim.cx;
    int dim_y = dst.dim.cy;

    for( int y = 0; y < (dim_y - 1); y += 2 )
    {
        uint16_t* dst_line0 = img::get_line_start<uint16_t>( dst, y + 0 );
        uint16_t* dst_line1 = img::get_line_start<uint16_t>( dst, y + 1 );

        wb_line_c_16bit(dst_line0, dst_line0, dim_x, mul_even_0, mul_even_1 );
        wb_line_c_16bit(dst_line1, dst_line1, dim_x, mul_odd_0, mul_odd_1 );
    } 
    if( dim_y % 2 == 1 )
    {
        int y = dim_y - 1;
        uint16_t* dst_line0 = img::get_line_start<uint16_t>( dst, y + 0 );

        wb_line_c_16bit(dst_line0, dst_line0, dim_x, mul_even_0, mul_even_1 );
    }
}

}

void		img_filter::whitebalance::detail::apply_wb_by16_c( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb )
{
    if( wb_r == 64 && wb_gr == 64 && wb_b == 64 && wb_gb == 64 ) {
        return;
    }

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGGR16:	wb_image_c_16bit( dst, wb_b, wb_gb, wb_gr, wb_r ); break;
    case img::fourcc::GBRG16:	wb_image_c_16bit( dst, wb_gb, wb_b, wb_r, wb_gr ); break;
    case img::fourcc::GRBG16:	wb_image_c_16bit( dst, wb_gr, wb_r, wb_b, wb_gb ); break;
    case img::fourcc::RGGB16:	wb_image_c_16bit( dst, wb_r, wb_gr, wb_gb, wb_b ); break;
    default:
        return;
    };
}
