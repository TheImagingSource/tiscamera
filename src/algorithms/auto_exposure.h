/*
 * Copyright 2018 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "algorithms/auto_alg_params.h"
#include "pid_controller.h"

namespace auto_alg::impl
{
	struct gain_exposure_iris_values
	{
		int		exposure;
		float	gain;
		int		iris;
	};

gain_exposure_iris_values	calc_auto_gain_exposure_iris( float brightness,
                                                          float reference_value,
                                                          const auto_alg::property_cont_gain& gain_desc,
                                                          const auto_alg::property_cont_exposure& exposure_desc,
                                                          const auto_alg::property_cont_iris& iris_desc );
int							calc_auto_pwm_iris( float corrected_brightness,
                                                int reference_value,
                                                const auto_alg::property_cont_iris& iris_desc,
                                                detail::pid_controller& iris_controller );
}
