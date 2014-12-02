
#include "auto_exposure.h"

#include "adjust_gain_db_table.h"

#include <math.h>

namespace
{

static const unsigned int dist_mid = 100;

static unsigned int calc_dist( unsigned int reference_val, unsigned int brightness_val )
{
	if( brightness_val == 0 )
		brightness_val = 1;
	return (reference_val * dist_mid) / brightness_val;
}

static unsigned int calc_exposure( unsigned int dist, int exposure, const auto_alg::property_cont_exposure& range )
{
	dist = ( dist + (dist_mid*2) ) / 3;
	exposure = ( exposure * dist ) / dist_mid;

	// If we do not do a significant change (on the sensor-scale), don't change anything
	// This should avoid pumping caused by an abrupt brightness change caused by a small value change
	if( abs(exposure - range.val) < (range.granularity/2) )
		return range.val;

	return CLIP( exposure, range.min, range.max );
}

 // this is a bit curious, steps to double brightness is 3 / 217687e-6 = 13,78.. and not 301 like we assume for gain_db cameras ...
//static unsigned int calc_gain( unsigned int dist, int gain, const auto_alg::property_cont_gain& range )
//{
//	if( dist >= dist_mid )	// when we have to reduce, we reduce it faster
//		dist = (dist + (dist_mid * 2)) / 3;	// this dampens the change in dist factor
//
//	double val = log( dist / (double)dist_mid ) / log( 2.0f );
//
//	gain += (int)(val * range.steps_to_double_brightness);
//
//	return CLIP( gain, range.min, range.max );
//}

static unsigned int calc_gain_db( unsigned int dist, int gain, const auto_alg::property_cont& range )
{
	if( dist >= dist_mid )	// when we have to reduce, we reduce it faster
		dist = (dist + (dist_mid * 2)) / 3;	// this dampens the change in dist factor

	float val = log( dist / (float) dist_mid ) / log( 2.0f ) * 301.f;
	gain += (int) val;

	//gain += adjust_gain_db( dist );
	return CLIP( gain, range.min, range.max );
}

static unsigned int calc_gain_default( unsigned int dist, int gain, const auto_alg::property_cont& range )
{
	if( dist >= dist_mid )	// when we have to reduce, we reduce it faster
		dist = (dist + (dist_mid * 2)) / 3;	// this dampens the change in dist factor

	if( gain == 0 )
		gain = 1;
	gain = (gain * dist) / dist_mid;

	return CLIP( gain, range.min, range.max );
}

static unsigned int calc_gain( unsigned int dist, int gain, const auto_alg::property_cont_gain& range )
{
	if( range.is_db_gain )
		return calc_gain_db( dist, gain, range );
	else
		return calc_gain_default( dist, gain, range );
}

static int calc_iris( int dist, int iris, const auto_alg::property_cont& range )
{
	//if( dist >= dist_mid )	// when we have to reduce, we reduce it faster
	dist = (dist + (dist_mid * 3)) / 4;	// this dampens the change in dist factor
	//dist = (dist + (dist_mid * 3)) / 3;

	if( iris == 0 )
		iris = 1;
	iris = (iris * dist) / dist_mid;

	return CLIP( iris, range.min, range.max );
}

}

auto_alg::gain_exposure_iris_values		auto_alg::calc_auto_gain_exposure_iris( int brightness, int reference_value, const auto_alg::property_cont_gain& gain_desc,
	const auto_alg::property_cont_exposure& exposure_desc, const auto_alg::property_cont& iris_desc )
{
	auto_alg::gain_exposure_iris_values rval = {};
	if( exposure_desc.do_auto )
		rval.exposure = CLIP( exposure_desc.val, exposure_desc.min, exposure_desc.max );
	else
		rval.exposure = exposure_desc.val;

	if( gain_desc.do_auto )
		rval.gain = CLIP( gain_desc.val, gain_desc.min, gain_desc.max );
	else
		rval.gain = gain_desc.val;

	if( iris_desc.do_auto )
		rval.iris = CLIP( iris_desc.val, iris_desc.min, iris_desc.max );
	else
		rval.iris = iris_desc.val;

	int gain = rval.gain;
	int exposure = rval.exposure;
	int iris = iris_desc.val;;

	int dist = calc_dist( reference_value, brightness );
	if( dist < 96 || dist > 104 )
	{
		int tmp_gain = 0;
		if( gain_desc.do_auto )
		{
			// reduce gain, if possible
			tmp_gain = calc_gain( dist, gain, gain_desc );
			if( tmp_gain < gain )
			{
				rval.gain = tmp_gain;
				return rval;
			}
		}

		int tmp_iris = 0;
		if( iris_desc.do_auto )
		{
			// iris
			tmp_iris = calc_iris( dist, iris, iris_desc );
			if( tmp_iris < iris )
			{
				rval.iris = tmp_iris;
				return rval;
			}
		}

		int tmp_exposure = 0;
		if( exposure_desc.do_auto )
		{
			// exposure
			tmp_exposure = calc_exposure( dist, exposure, exposure_desc );
			if( tmp_exposure != exposure )
			{
				rval.exposure = tmp_exposure;
				return rval;
			}
		}

		if( iris_desc.do_auto )
		{
			// when gain is in a sweet spot, or cannot be increased anymore
			if( tmp_iris != iris /*&& exposure >= exposure_desc.max*/ )
			{
				rval.iris = tmp_iris;
				return rval;
			}
		}

		if( gain_desc.do_auto )
		{
			// when exposure is in a sweet spot, or cannot be increased anymore
			if( tmp_gain != gain /*&& exposure >= exposure_desc.max*/ )
			{
				rval.gain = tmp_gain;
				return rval;
			}
		}
	}



	if( exposure_desc.do_auto && gain_desc.do_auto &&
		gain > gain_desc.min && exposure < exposure_desc.max			// we can reduce gain, because we can increase exposure
		)
	{
		rval.exposure = CLIP( (exposure * 105) / 100, exposure_desc.min, exposure_desc.max );	// increase exposure by 5%
		return rval;
	}

	if( exposure_desc.do_auto && iris_desc.do_auto &&
		iris > iris_desc.min && exposure < exposure_desc.max			// we can reduce iris, because we can increase exposure
		)
	{
		rval.exposure = CLIP( (exposure * 105) / 100, exposure_desc.min, exposure_desc.max );	// increase exposure by 5%
		return rval;
	}


	if( gain_desc.do_auto &&  iris_desc.do_auto &&
		gain > gain_desc.min && iris < iris_desc.max			// we can reduce gain, because we can increase iris
		)
	{
		rval.iris = CLIP( (iris * 105) / 100, iris_desc.min, iris_desc.max );	// increase iris by 5%
		return rval;
	}
	return rval;
}

int auto_alg::calc_auto_pwm_iris( float corrected_brightness, unsigned int reference_value, const auto_alg::property_cont_iris& iris_desc, pid_controller& iris_controller )
{
	// Don't calculate with too slow frame rates, the iris might go too fast
	float fps_ceil = (float)MIN( iris_desc.camera_fps, 60 );

	float e = reference_value - corrected_brightness;
	float y = iris_controller.step( e, fps_ceil );

	int iris_mid = (iris_desc.max - iris_desc.min) / 2;
	int iris_val = (int)(iris_mid - y);

	return CLIP(iris_val, iris_desc.min, iris_desc.max);
}
