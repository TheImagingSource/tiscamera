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

#ifndef _AUTOFOCUS_H_
#define _AUTOFOCUS_H_

#include "algorithms/auto_alg_params.h"
#include "compiler_defines.h"
#include "img/image_transform_base.h"

#include <stdbool.h>

VISIBILITY_DEFAULT

namespace tcam::algorithms::focus
{
/* opaque object */
struct AutoFocus;
typedef struct AutoFocus AutoFocus;

/* @brief Create AutoFocus instance */
AutoFocus* autofocus_create(void);

/* @brief Destroy AutoFocus instance */
void autofocus_destroy(AutoFocus* focus);

/* @param pixel_dim dimension of a single pixel
                        usually 1x1, binning may change that
     */
bool autofocus_run(AutoFocus* focus,
                   uint64_t time_point,
                   const img::img_descriptor& img,
                   const auto_alg::auto_focus_params& state,
                   img::point offsets,
                   img::dim pixel_dim,
                   int& new_focus_vale);

/* @name autofocus_is_running */
/* @param focus - AutoFocus that shall be checked */
/* @return true if focus is active */
bool autofocus_is_running(AutoFocus* focus);

/* @name autofocus_end */
/* @brief Force stop */
void autofocus_end(AutoFocus* focus);

} // namespace tcam::algorithms::focus

VISIBILITY_POP

#endif /* _AUTOFOCUS_H_ */
