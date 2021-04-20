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


namespace auto_alg
{

namespace detail
{

class pid_controller
{
private:
    float _P, _I, _D;
    float _e_sum_limit;

    float _e_sum;

    float _e_prev;
    bool _e_prev_valid;


public:
    pid_controller(float p, float i, float d, float e_sum_limit);

    float step(float e, float fps);

    void reset(void);
};
} // namespace detail
} // namespace auto_alg
