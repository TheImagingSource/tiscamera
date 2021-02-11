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

#include "pid_controller.h"

using namespace algorithms::detail;

pid_controller::pid_controller(float p, float i, float d, float e_sum_limit)
    : _P(p), _I(i), _D(d), _e_sum_limit(e_sum_limit), _e_sum(0), _e_prev(0), _e_prev_valid(false)
{
}

void pid_controller::reset()
{
    _e_sum = 0;
    _e_prev_valid = false;
}

float pid_controller::step(float e, float fps)
{
    _e_sum += e;

    if (fps == 0.0f)
    {
        fps = 1.0f;
    }

    float p = _P * e;
    float i = _I * _e_sum / fps;
    float d = 0;

    if (_e_prev_valid)
    {
        d = _D * (e - _e_prev) / fps;
    }

    if (_e_sum > _e_sum_limit)
    {
        _e_sum = _e_sum_limit;
    }
    if (_e_sum < -_e_sum_limit)
    {
        _e_sum = -_e_sum_limit;
    }

    return p + i + d;
}
