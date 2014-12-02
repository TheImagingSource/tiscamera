
#pragma once

#include "auto_alg_params.h"
#include "pid_controller.h"

namespace auto_alg
{
	struct gain_exposure_iris_values
	{
		int	exposure;
		int	gain;
		int	iris;
	};

	gain_exposure_iris_values	calc_auto_gain_exposure_iris( int brightness, int reference_value, const auto_alg::property_cont_gain& gain_desc, 
									const auto_alg::property_cont_exposure& exposure_desc, const auto_alg::property_cont& iris_desc );
	int							calc_auto_pwm_iris( float corrected_brightness, unsigned int reference_value, const auto_alg::property_cont_iris& iris_desc, pid_controller& iris_controller );
};
