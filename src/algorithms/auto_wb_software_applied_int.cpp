
#include "auto_alg.h"

#include "auto_wb_config.h"

#include <algorithm>

namespace 
{
	using namespace auto_alg::impl::wb;

    using std::abs;


    // This result in that at least one of r/g/b is WB_IDENTITY
    static rgb_tripel   adjust_to_identity( rgb_tripel wb ) noexcept
    {
        // This result in that at least one of r/g/b is WB_IDENTITY
        const int dist0 = std::min( { wb.r - WB_IDENTITY, wb.g - WB_IDENTITY, wb.b - WB_IDENTITY } );
        if( dist0 > 0 ) {
            wb.r -= dist0;
            wb.g -= dist0;
            wb.b -= dist0;
        }
        return wb;
    }

static int calc_step( int /*delta*/ ) noexcept
{
    return 1;
}

static bool wb_auto_step( const rgb_tripel& clr, rgb_tripel& wb ) noexcept
{
    int avg = ((clr.r + clr.g + clr.b) / 3);
    int dr = avg - clr.r;
    int dg = avg - clr.g;
    int db = avg - clr.b;

    if( abs(dr) < BREAK_DIFF && abs(dg) < BREAK_DIFF && abs(db) < BREAK_DIFF )
    {
        wb.r = clip_to_wb( wb.r );
        wb.g = clip_to_wb( wb.g );
        wb.b = clip_to_wb( wb.b );
        return true;
    }

    if( (clr.r > avg) && (wb.r > WB_IDENTITY) ) 
    {
        wb.r -= calc_step(dr);
    }
    if ((clr.g > avg) && (wb.g > WB_IDENTITY))
    {
        wb.g -= calc_step(dg);
    }
    if ((clr.b > avg) && (wb.b > WB_IDENTITY))
    {
        wb.b -= calc_step(db);
    }

    if( (clr.r < avg) && (wb.r < WB_MAX) )
    {
        wb.r += calc_step(dr);
    }
    if( (clr.g < avg) && (wb.g < WB_MAX) )
    {
        wb.g += calc_step(dg);
    }
    if( (clr.b < avg) && (wb.b < WB_MAX) )
    {
        wb.b += calc_step(db);
    }

    wb = adjust_to_identity( wb );
    return false;
}

static rgb_tripel simulate_whitebalance( const auto_alg::impl::auto_sample_points& data, const rgb_tripel& wb ) noexcept
{
    rgb_tripel result = { 0, 0, 0 };
    rgb_tripel result_near_gray = { 0, 0, 0 };
    int count_near_gray = 0;

    for( int i = 0; i < data.cnt; ++i )
    {
        const auto src_pix = data.samples[i].to_pixel();

        auto_alg::impl::pixel pix;
        pix.r = (uint8_t)clip_to_wb( src_pix.r * wb.r / WB_IDENTITY );
        pix.g = (uint8_t)clip_to_wb( src_pix.g * wb.g / WB_IDENTITY );
        pix.b = (uint8_t)clip_to_wb( src_pix.b * wb.b / WB_IDENTITY );

        result += pix;

        if( is_near_gray( pix ) )
        {
            result_near_gray += pix;
            count_near_gray += 1;
        }
    }

    float near_gray_amount = count_near_gray / (float)data.cnt;

    if( near_gray_amount < NEARGRAY_REQUIRED_AMOUNT )
    {
        result /= data.cnt;
        return result;
    }
    else
    {
        result_near_gray /= count_near_gray;
        return result_near_gray;
    }
}

}


auto auto_alg::impl::auto_whitebalance_soft( const auto_alg::impl::auto_sample_points& data, const wb_channel_factors& wb_in_factors ) -> auto_whitebalance_result
{
    rgb_tripel wb = to_rgb_tripel( wb_in_factors );

    // for values < identity set them to identity
    if( wb.r < WB_IDENTITY ) wb.r = WB_IDENTITY;
    if( wb.g < WB_IDENTITY ) wb.g = WB_IDENTITY;
    if( wb.b < WB_IDENTITY ) wb.b = WB_IDENTITY;

    wb = adjust_to_identity( wb );

	if( data.cnt == 0 ) {
		return { false, to_wb_channel_factors( wb ) };
	}

    unsigned int steps = 0;
    while( steps++ < MAX_STEPS )
    {
        rgb_tripel tmp = simulate_whitebalance( data, wb );
        if( wb_auto_step( tmp, wb ) ) {
			return { true, to_wb_channel_factors( wb ) };
        }
    }
    wb.r = clip_to_wb( wb.r );
    wb.g = clip_to_wb( wb.g );
    wb.b = clip_to_wb( wb.b );

	return { false, to_wb_channel_factors( wb ) };
}
