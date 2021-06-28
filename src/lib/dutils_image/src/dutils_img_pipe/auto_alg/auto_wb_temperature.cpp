
#include "auto_wb_temperature.h"

#include <cmath>

using namespace auto_alg;

namespace 
{

auto_alg::wb_channel_factors GetFactorsFromTemperature( int temperature, const wb_channel_factors* arr )
{
	if( temperature < 2500 ) {
		temperature = 2500;
	}
	if( temperature > 10000 ) {
		temperature = 10000;
	}
	int index = (temperature - 2500) / 100;
	return arr[index];
}


static int clip( int x )
{
	if( x > 255 ) return 255;
	if( x < 0 ) return 0;
	return x;
}


static int SimulateWhiteBalance( const auto_alg::impl::auto_sample_points& points, wb_channel_factors factors )
{
	const float min_c = 0.925f;
	const float max_c = 1.0f / min_c;

	int whiteCnt = 0;

	for( int i = 0; i < points.cnt; ++i )
	{
		const auto pix = points.samples[i].to_pixel();

		const int r = clip( (int)((float)pix.r * factors.r) );
		const int g = clip( (int)((float)pix.g * factors.g) );
		const int b = clip( (int)((float)pix.b * factors.b) );
		const int y = ((r * 79 + g * 150 + b * 27) >> 8);
		if( y > 50 && y < 240 )
		{

			float qrg = (float)r / (float)g;
			float qbg = (float)b / (float)g;
			float qrb = (float)r / (float)b;

			if( qrb > min_c && qrb < max_c &&
				qbg > min_c && qbg < max_c &&
				qrg > min_c && qrg < max_c )
			{
				whiteCnt++;
			}
		}
	}
	return whiteCnt;
}

}

int		auto_alg::impl::calc_temperature_for_pixels( const auto_alg::impl::auto_sample_points& points, int min_temperature, int max_temperature, const wb_channel_factors* arr )
{
	int maxWhiteTemp = -1;
	float maxWhiteValue = -1;

	//  Get temperature with most potentially white pixels
	for( int temperature_iter = min_temperature; temperature_iter < max_temperature; temperature_iter += 100 )
	{
		wb_channel_factors wb_factors = GetFactorsFromTemperature( temperature_iter, arr );
		const int white_count = SimulateWhiteBalance( points, wb_factors );
		const float whiteValue = (float)white_count;

		// normal distribution around center
		if( whiteValue > maxWhiteValue )
		{
			maxWhiteValue = whiteValue;
			maxWhiteTemp = temperature_iter;
		}
	}
	return maxWhiteTemp;
}

int auto_alg::impl::calc_temperature_auto_step( const auto_sample_points& points, int current_temperature, int auto_min_temp, int auto_max_temp, const wb_channel_factors* arr )
{
	int calc_temp = calc_temperature_for_pixels( points, auto_min_temp, auto_max_temp, arr );
	// choose last valid temperature or if 
	// no white pixels were detected
	if( calc_temp == -1 )
	{
		if( current_temperature < auto_min_temp ||
			current_temperature > auto_max_temp )
		{
			int center = (auto_min_temp - auto_max_temp) / 2 + auto_min_temp;
			calc_temp = center;
		}
		else
		{
			calc_temp = current_temperature;
		}
	}

	// fade current temperature to new temperature
	if( (calc_temp - current_temperature) != 0 )
	{
		current_temperature += (calc_temp - current_temperature) / 3;
	}

	return current_temperature;
}

wb_channel_factors	auto_alg::impl::calc_whitebalance_values( int temperature, const wb_channel_factors* arr )
{
	return GetFactorsFromTemperature( temperature, arr );

}
