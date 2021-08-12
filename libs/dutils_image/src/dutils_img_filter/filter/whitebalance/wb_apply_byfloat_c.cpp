
#include "wb_apply.h"

namespace {

FORCEINLINE float wb_pixel_byf_c( float pixel, float factor )
{
    float val = pixel * factor;
    return val > 1.f ? 1.f : val;
}

void wb_line_byf_c( float* dst_line, int dim_x, float factor_00, float factor_01 )
{
    int x = 0;
    for( ; x < (dim_x - 1); x += 2 )
    {
        dst_line[x + 0] = wb_pixel_byf_c( dst_line[x + 0], factor_00 );
        dst_line[x + 1] = wb_pixel_byf_c( dst_line[x + 1], factor_01 );
    }
    if( x == (dim_x - 1) )
    {
        dst_line[x] = wb_pixel_byf_c( dst_line[x], factor_00 );
    }
}

void	wb_image_byf_c( img::img_descriptor dst, float factor_00, float factor_01, float factor_10, float factor_11 )
{
    int y = 0;
    for( ; y < (dst.dim.cy - 1); y += 2 )
    {
        float* dst_line0 = img::get_line_start<float>( dst, y + 0 );
        wb_line_byf_c( dst_line0, dst.dim.cx, factor_00, factor_01 );		// even line

        float* dst_line1 = img::get_line_start<float>( dst, y + 1 );
        wb_line_byf_c( dst_line1, dst.dim.cx, factor_10, factor_11 );		// odd line
    }
    if( y == (dst.dim.cy - 1) )
    {
        float* dst_line0 = img::get_line_start<float>( dst, y + 0 );

        wb_line_byf_c( dst_line0, dst.dim.cx, factor_00, factor_01 );
    }
}

}

void		img_filter::whitebalance::detail::apply_wb_byfloat_c( const img::img_descriptor& dst, const apply_params& params )
{
    if( params.wb_rr == 1.f && params.wb_gr == 1.f && params.wb_bb == 1.f && params.wb_gb == 1.f ) {
        return;
    }

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGGRFloat:	wb_image_byf_c( dst, params.wb_bb, params.wb_gb, params.wb_gr, params.wb_rr ); break;
    case img::fourcc::GBRGFloat:	wb_image_byf_c( dst, params.wb_gb, params.wb_bb, params.wb_rr, params.wb_gr ); break;
    case img::fourcc::GRBGFloat:	wb_image_byf_c( dst, params.wb_gr, params.wb_rr, params.wb_bb, params.wb_gb ); break;
    case img::fourcc::RGGBFloat:	wb_image_byf_c( dst, params.wb_rr, params.wb_gr, params.wb_gb, params.wb_bb ); break;
    default:
        return;
    };
}