
#include "wb_apply.h"

namespace {

FORCEINLINE uint8_t wb_pixel_c_8bit( uint8_t pixel, uint8_t factor )
{
    int val = (pixel * factor) / 64;
    return  val > 0xFF ? 0xFF : uint8_t( val );
}

void wb_line_c_8bit( uint8_t* dst_line, uint8_t* src_line, int dim_x, uint8_t factor_00, uint8_t factor_01 )
{
    int x = 0;
    for( ; x < (dim_x - 1); x += 2 )
    {
        unsigned int v0 = wb_pixel_c_8bit( src_line[x + 0], factor_00 );
        unsigned int v1 = wb_pixel_c_8bit( src_line[x + 1], factor_01 );
        *((uint16_t*)(dst_line + x)) = (uint16_t)(v1 << 8 | v0);
    }
    if( x == (dim_x - 1) )
    {
        dst_line[x] = wb_pixel_c_8bit( src_line[x], factor_00 );
    }
}

void	wb_image_c_8bit( img::img_descriptor dst, uint8_t factor_00, uint8_t factor_01, uint8_t factor_10, uint8_t factor_11 )
{
    int y = 0;
    for( ; y < (dst.dim.cy - 1); y += 2 )
    {
        uint8_t* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );
        uint8_t* dst_line1 = img::get_line_start<uint8_t>( dst, y + 1 );

        wb_line_c_8bit( dst_line0, dst_line0, dst.dim.cx, factor_00, factor_01 );		// even line
        wb_line_c_8bit( dst_line1, dst_line1, dst.dim.cx, factor_10, factor_11 );		// odd line
    }
    if( y == (dst.dim.cy - 1) )
    {
        uint8_t* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );

        wb_line_c_8bit( dst_line0, dst_line0, dst.dim.cx, factor_00, factor_01 );
    }
}

}

void		img_filter::whitebalance::detail::apply_wb_by8_c( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb )
{
    if( wb_r == 64 && wb_gr == 64 && wb_b == 64 && wb_gb == 64 ) {
        return;
    }

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGGR8:	wb_image_c_8bit( dst, wb_b, wb_gb, wb_gr, wb_r ); break;
    case img::fourcc::GBRG8:	wb_image_c_8bit( dst, wb_gb, wb_b, wb_r, wb_gr ); break;
    case img::fourcc::GRBG8:	wb_image_c_8bit( dst, wb_gr, wb_r, wb_b, wb_gb ); break;
    case img::fourcc::RGGB8:	wb_image_c_8bit( dst, wb_r, wb_gr, wb_gb, wb_b ); break;
    default:
        break;
    };
}