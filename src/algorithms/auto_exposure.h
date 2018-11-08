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

#include "pid_controller.h"

namespace algorithms
{

// simple definition of an int property
struct property_cont
{
    int  min;
    int  max;

    int  val;
    bool do_auto;
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


struct gain_exposure_iris_values
{
    int     exposure;
    int     gain;
    int     iris;
};


/// @brief calculate new exposure/gain/iris values
/// @param brightness - brightness the current image has
/// @param reference_value - optimal image brightness that should be achieved
/// @param gain_desc - description of the current gain settings
/// @param exposure_desc - description of the current exposure settings
/// @param iris_desc - description of the current iris settings
/// @return struct containing the calculated setting
///
///
gain_exposure_iris_values calc_auto_gain_exposure_iris (int brightness,
                                                        int reference_value,
                                                        const algorithms::property_cont_gain& gain_desc,
                                                        const algorithms::property_cont_exposure& exposure_desc,
                                                        const algorithms::property_cont& iris_desc);


int calc_auto_pwm_iris (float corrected_brightness,
                        unsigned int reference_value,
                        const algorithms::property_cont_iris& iris_desc,
                        detail::pid_controller& iris_controller);

} /* namespace algorithms */
