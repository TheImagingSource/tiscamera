

#pragma once

#include "auto_alg.h"

namespace auto_alg::impl
{
	// calculates a wb-temperature based on the previous temperature, so that changes smaller
	int			calc_temperature_auto_step( const auto_sample_points& points, int current_temperature, int auto_min_temp, int auto_max_temp, const wb_channel_factors* arr );

	// calculates the current temperature of the passed points, this may -1 when the algorithm cannot determine the current temperature
	int			calc_temperature_for_pixels( const auto_sample_points& points, int min_temperature, int max_temperature, const wb_channel_factors* arr );

	wb_channel_factors	calc_whitebalance_values( int temperature, const wb_channel_factors* arr );
}