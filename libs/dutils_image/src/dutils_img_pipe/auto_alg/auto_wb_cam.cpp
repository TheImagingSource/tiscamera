
#include "auto_alg.h"

#include "auto_wb_config.h"

#include <cmath>

namespace 
{
    using namespace auto_alg::impl::wb;

    using std::abs;

static int calc_step( int delta ) noexcept
{
    int step = abs(delta) / 4;
    if( step < 1 ) step = 1;

    return step;
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

    if( (wb.r > WB_IDENTITY) && (wb.g > WB_IDENTITY) && (wb.b > WB_IDENTITY) )
    {
        wb.r -= 1;
        wb.g -= 1;
        wb.b -= 1;
    }
    return false;
}

static rgb_tripel calc_wb_for_frame( const auto_alg::impl::auto_sample_points& data )
{
    rgb_tripel result = { 0, 0, 0 };
    rgb_tripel result_near_gray = { 0, 0, 0 };
    int count_near_gray = 0;

    for( int i = 0; i < data.cnt; ++i )
    {
        auto pix = data.samples[i].to_pixel();

        result += pix;

        if( is_near_gray( pix ) )
        {
            result_near_gray += pix;
            count_near_gray += 1;
        }
    }

    const float near_gray_amount = count_near_gray / (float)data.cnt;

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


auto auto_alg::impl::auto_whitebalance_cam( const auto_alg::impl::auto_sample_points& data, const wb_channel_factors& wb_in_factors ) -> auto_whitebalance_result
{
	rgb_tripel wb = to_rgb_tripel( wb_in_factors );
	rgb_tripel old_wb = wb;

	if( wb.r < WB_IDENTITY ) wb.r = WB_IDENTITY;
	if( wb.g < WB_IDENTITY ) wb.g = WB_IDENTITY;
	if( wb.b < WB_IDENTITY ) wb.b = WB_IDENTITY;
	if( old_wb.r != wb.r || old_wb.g != wb.g || old_wb.b != wb.b ) {
		return { false, to_wb_channel_factors( wb ) };
	}

	while( (wb.r > WB_IDENTITY) && (wb.g > WB_IDENTITY) && (wb.b > WB_IDENTITY) )
	{
		wb.r -= 1;
		wb.g -= 1;
		wb.b -= 1;
	}

	if( data.cnt == 0 ) {
		return { false, to_wb_channel_factors( wb ) };
	}

    //unsigned int steps = 0;
    //while( steps++ < MAX_STEPS )
    {
        rgb_tripel tmp = calc_wb_for_frame( data );
        if( wb_auto_step( tmp, wb ) ) {
			return { true, to_wb_channel_factors( wb ) };
        }
    }
    wb.r = clip_to_wb( wb.r );
    wb.g = clip_to_wb( wb.g );
    wb.b = clip_to_wb( wb.b );

    // Return true if nothing has been changed (e.g. all values clipped),
    // so that the one push is terminated
    bool res = (wb.r == old_wb.r) && (wb.g == old_wb.g) && (wb.b == old_wb.b);
	return { res, to_wb_channel_factors( wb ) };
}
