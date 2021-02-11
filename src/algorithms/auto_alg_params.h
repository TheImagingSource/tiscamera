/*
 * Copyright 2019 The Imaging Source Europe GmbH
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

#include "img/image_helper_types.h"


namespace auto_alg
{

struct auto_focus_params
{
    int device_focus_val; // the currently set focus value, this should be the actual value, that is present in the device

    bool is_end_cmd; // when set, the auto_focus code stops the current auto run
    bool
        is_run_cmd; // when set, the auto_focus code resets and takes the run_cmd_params to init itself
    struct run_cmd_param_struct
    {
        img::rect roi; // user roi, must { 0, 0, 0, 0 } to be ignored
        int focus_range_min; // minimum focus range as provided by the device/user
        int focus_range_max; // maximum  ^^
        int focus_device_speed; // device speed, currently set to 500
        int auto_step_divisor; // supplied by the device, otherwise currently 4
        bool suggest_sweep; // should be default false, otherwise suggested by the device

        int focus_min_move_wait_in_ms; // minimum ms to wait for a focus movement to end
    } run_cmd_params;
};

} /* namespace auto_alg */
