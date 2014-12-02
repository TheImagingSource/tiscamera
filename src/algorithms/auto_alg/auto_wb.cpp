

#include "auto_alg.h"

#include "auto_wb_config.h"

using namespace auto_alg;

#define abs(x) (((x) < 0) ? -(x) : (x))

namespace
{

bool wb_auto_step( const rgb_tripel& clr, rgb_tripel& wb )
{
	int avg = ((clr.r + clr.g + clr.b) / 3);
	int dr = avg - clr.r;
	int dg = avg - clr.g;
	int db = avg - clr.b;

	if( abs(dr) < BREAK_DIFF && abs(dg) < BREAK_DIFF && abs(db) < BREAK_DIFF )
	{
		wb.r = clip<WB_MAX>( wb.r );
		wb.g = clip<WB_MAX>( wb.g );
		wb.b = clip<WB_MAX>( wb.b );
		return true;
	}

	if( (clr.r > avg) && (wb.r > WB_IDENTITY) )
	{
		wb.r -= 1;
	}
	if ((clr.g > avg) && (wb.g > WB_IDENTITY))
	{
		wb.g -= 1;
	}
	if ((clr.b > avg) && (wb.b > WB_IDENTITY))
	{
		wb.b -= 1;
	}

	if( (clr.r < avg) && (wb.r < WB_MAX) )
	{
		wb.r += 1;
	}
	if( (clr.g < avg) && (wb.g < WB_MAX) )
	{
		wb.g += 1;
	}
	if( (clr.b < avg) && (wb.b < WB_MAX) )
	{
		wb.b += 1;
	}

	if( (wb.r > WB_IDENTITY) && (wb.g > WB_IDENTITY) && (wb.b > WB_IDENTITY) )
	{
		wb.r -= 1;
		wb.g -= 1;
		wb.b -= 1;
	}
	return false;
}

bool is_near_gray( unsigned int r, unsigned int g, unsigned int b )
{
	unsigned int brightness = calc_brightness_from_clr_avg( r, g, b );
	if( brightness < NEARGRAY_MIN_BRIGHTNESS ) return false;
	if( brightness > NEARGRAY_MAX_BRIGHTNESS ) return false;

	unsigned int deltaR = abs(int(r) - int(brightness));
	unsigned int deltaG = abs(int(g) - int(brightness));
	unsigned int deltaB = abs(int(b) - int(brightness));

	float devR = deltaR / (float)brightness;
	float devG = deltaG / (float)brightness;
	float devB = deltaB / (float)brightness;

	return (devR < NEARGRAY_MAX_COLOR_DEVIATION) && (devG < NEARGRAY_MAX_COLOR_DEVIATION) && (devB < NEARGRAY_MAX_COLOR_DEVIATION);
}

rgb_tripel simulate_whitebalance( const auto_sample_points& data, const auto_alg::by8_mtx& clr, const rgb_tripel& wb, bool enable_near_gray )
{
	rgb_tripel result = { 0, 0, 0 };
	rgb_tripel result_near_gray = { 0, 0, 0 };
	int count_near_gray = 0;

    if( data.cnt == 0 )
        return wb;

	for( int i = 0; i < data.cnt; ++i )
	{
		int r = clip<WB_MAX>( data.samples[i].r * wb.r / WB_IDENTITY );
		int g = clip<WB_MAX>( data.samples[i].g * wb.g / WB_IDENTITY );
		int b = clip<WB_MAX>( data.samples[i].b * wb.b / WB_IDENTITY );

		result.r += r;
		result.g += g;
		result.b += b;

		if( is_near_gray( r, g, b ) )
		{
			result_near_gray.r += r;
			result_near_gray.g += g;
			result_near_gray.b += b;
			count_near_gray += 1;
		}
	}

	float near_gray_amount = count_near_gray / (float)data.cnt;

	if( (near_gray_amount < NEARGRAY_REQUIRED_AMOUNT) || !enable_near_gray )
	{
		result.r /= data.cnt;
		result.g /= data.cnt;
		result.b /= data.cnt;
		return result;
	}
	else
	{
		result_near_gray.r /= count_near_gray;
		result_near_gray.g /= count_near_gray;
		result_near_gray.b /= count_near_gray;
		return result_near_gray;
	}
}

rgb_tripel simulate_whitebalance( const auto_sample_points& data, const rgb_tripel& wb, bool enable_near_gray )
{
	rgb_tripel result = { 0, 0, 0 };
	rgb_tripel result_near_gray = { 0, 0, 0 };
	int count_near_gray = 0;

    if( data.cnt == 0 )
        return wb;

	for( int i = 0; i < data.cnt; ++i )
	{
		unsigned int r = clip<WB_MAX>( data.samples[i].r * wb.r / WB_IDENTITY );
		unsigned int g = clip<WB_MAX>( data.samples[i].g * wb.g / WB_IDENTITY );
		unsigned int b = clip<WB_MAX>( data.samples[i].b * wb.b / WB_IDENTITY );

		result.r += r;
		result.g += g;
		result.b += b;

		if( is_near_gray( r, g, b ) )
		{
			result_near_gray.r += r;
			result_near_gray.g += g;
			result_near_gray.b += b;
			count_near_gray += 1;
		}
	}

	float near_gray_amount = count_near_gray / (float)data.cnt;

	if( (near_gray_amount < NEARGRAY_REQUIRED_AMOUNT) || !enable_near_gray )
	{
		result.r /= data.cnt;
		result.g /= data.cnt;
		result.b /= data.cnt;
		return result;
	}
	else
	{
		result_near_gray.r /= count_near_gray;
		result_near_gray.g /= count_near_gray;
		result_near_gray.b /= count_near_gray;
		return result_near_gray;
	}
}

}


bool auto_alg::auto_whitebalance( const auto_sample_points& data, const auto_alg::by8_mtx& clr, rgb_tripel& wb )
{
	if( data.cnt == 0 )
		return false;

	rgb_tripel old_wb = wb;
	if( wb.r < WB_IDENTITY ) wb.r = WB_IDENTITY;
	if( wb.g < WB_IDENTITY ) wb.g = WB_IDENTITY;
	if( wb.b < WB_IDENTITY ) wb.b = WB_IDENTITY;
	if( old_wb.r != wb.r || old_wb.g != wb.g || old_wb.b != wb.b )
		return false;

	while( (wb.r > WB_IDENTITY) && (wb.g > WB_IDENTITY) && (wb.b > WB_IDENTITY) )
	{
		wb.r -= 1;
		wb.g -= 1;
		wb.b -= 1;
	}

	unsigned int steps = 0;
	while( steps++ < MAX_STEPS )
	{
		rgb_tripel tmp = simulate_whitebalance( data, clr, wb, true );
		if( wb_auto_step(tmp, wb) )
			return true;
	}
	wb.r = clip<WB_MAX>( wb.r );
	wb.g = clip<WB_MAX>( wb.g );
	wb.b = clip<WB_MAX>( wb.b );

	return false;
}

unsigned int	auto_alg::calc_resulting_brightness( const auto_sample_points& samples, const by8_mtx& clr, const rgb_tripel& wb )
{
	rgb_tripel tmp = simulate_whitebalance( samples, clr, wb, false );

	return calc_brightness_from_clr_avg( tmp );
}
