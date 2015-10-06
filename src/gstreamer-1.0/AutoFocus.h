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

#include <stdbool.h>
#include "image_transform_base.h"

//#include "tcam_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /* opaque object */
    struct AutoFocus;
    typedef struct AutoFocus AutoFocus;

    /* @brief Create AutoFocus instance */
    AutoFocus* autofocus_create (void);

    /* @brief Destroy AutoFocus instance */
    void autofocus_destroy (AutoFocus* focus);

    /* @brief Define parameter for next auto focus run */
    /* @param focus - AutoFocus instance to use */
    /* @param focus_val - current focus value */
    /* @param min - allowed minimum focus value */
    /* @param max - allowed maximum focus value */
    /* @param roi - region of interest that shall be focused instead of whole image */
    /* @param speed - */
    /* @param auto_step_divisor - */
    /* @param suggest_sweep - */
    void autofocus_run (AutoFocus* focus,
                        int focus_val,
                        int min,
                        int max,
                        RECT roi,
                        int speed,
                        int auto_step_divisor,
                        bool suggest_sweep);

    /* @name autofocus_analyze_frame */
    /* @param focus - AutoFocus instance to use */
    /* @param img - image description that shall be analyzed */
    /* @param offsets */
    /* @param binning_value */
    /* @param new_focus_value - will be set to new focus value */
    /* @return true if new_focus_value has been set */
    bool autofocus_analyze_frame (AutoFocus* focus,
                                  img_descriptor img,
                                  POINT offsets,
                                  int binning_value,
                                  int* new_focus_value);

    /* @name autofocus_is_running */
    /* @param focus - AutoFocus that shall be checked */
    /* @return true if focus is active */
    bool autofocus_is_running(AutoFocus* focus);

    /* @name autofocus_end */
    /* @brief Force stop */
    void autofocus_end (AutoFocus* focus);

    /* @name autofocus_update_focus */
    /* @brief Tell te auto focus process the new focus value */
    void autofocus_update_focus (AutoFocus* focus, int new_focus_value);

#ifdef __cplusplus
}
#endif

#endif /* _AUTOFOCUS_H_ */
