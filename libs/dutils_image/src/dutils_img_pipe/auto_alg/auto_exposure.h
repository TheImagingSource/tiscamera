
#pragma once

#include <dutils_img_pipe/auto_alg_params.h>
#include "pid_controller.h"

namespace auto_alg::impl
{
	struct gain_exposure_iris_values
	{
		int		exposure;
		float	gain;
		int		iris;
	};

	gain_exposure_iris_values	calc_auto_gain_exposure_iris( float brightness, float reference_value, const auto_alg::property_cont_gain& gain_desc, 
									const auto_alg::property_cont_exposure& exposure_desc, const auto_alg::property_cont_iris& iris_desc );
	int							calc_auto_pwm_iris( float corrected_brightness, int reference_value, const auto_alg::property_cont_iris& iris_desc, detail::pid_controller& iris_controller );
}
