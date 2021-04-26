/*
 * Copyright 2014 The Imaging Source Europe GmbH
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

#include "AutoFocus.h"

#include "auto_focus.h"

#include <stdbool.h>

namespace tcam::algorithms::focus
{

AutoFocus* autofocus_create(void)
{
    return reinterpret_cast<AutoFocus*>(new auto_alg::impl::auto_focus());
}


void autofocus_destroy(AutoFocus* focus)
{
    delete reinterpret_cast<auto_alg::impl::auto_focus*>(focus);
}


bool autofocus_run(AutoFocus* focus,
                   uint64_t time_point,
                   const img::img_descriptor& img,
                   const auto_alg::auto_focus_params& state,
                   img::point offsets,
                   img::dim pixel_dim,
                   int& new_focus_vale)
{
    return reinterpret_cast<auto_alg::impl::auto_focus*>(focus)->auto_alg_run(
        time_point, img, state, offsets, pixel_dim, new_focus_vale);
}


bool autofocus_is_running(AutoFocus* focus)
{
    return reinterpret_cast<auto_alg::impl::auto_focus*>(focus)->is_running();
}


void autofocus_end(AutoFocus* focus)
{
    reinterpret_cast<auto_alg::impl::auto_focus*>(focus)->reset();
}

} // namespace tcam::algorithms::focus
