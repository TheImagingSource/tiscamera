
#pragma once

#include "../by8/by8_base.h"

namespace auto_alg {
	// simple definition of an int property
	struct property_cont
	{
		int				min;
		int				max;

		int				val;
		bool			do_auto;
	};

	struct property_cont_exposure : public property_cont
	{
		int		granularity;
	};

    struct property_cont_gain : public property_cont
	{
		// Number of steps the gain has to be changed to achieve double image brightness
		double	steps_to_double_brightness;     // only needed for pwm iris
        bool    is_db_gain;                     // seemingly always true in the gigecam driver
	};

	struct property_cont_iris : public property_cont
	{
		// Currently configured frame rate
		double	camera_fps;

		bool	is_pwm_iris;
	};

	struct color_matrix_params
	{
		by8_transform::color_matrix		mtx;
		bool	enabled;
	};
};