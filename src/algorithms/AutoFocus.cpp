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

AutoFocus* autofocus_create (void)
{
    return reinterpret_cast<AutoFocus*>(new img::auto_focus());
}


void autofocus_destroy (AutoFocus* focus)
{
    delete reinterpret_cast<img::auto_focus*>(focus);
}


void autofocus_run (AutoFocus* focus,
                    int focus_val,
                    int min,
                    int max,
                    RECT roi,
                    int speed,
                    int auto_step_divisor,
                    bool suggest_sweep)
{
    reinterpret_cast<img::auto_focus*>(focus)->run(focus_val,
                                                   min,
                                                   max,
                                                   roi,
                                                   speed,
                                                   auto_step_divisor,
                                                   suggest_sweep);
}


bool autofocus_analyze_frame (AutoFocus* focus,
                              img_descriptor img,
                              POINT offsets,
                              int binning_value,
                              int* new_focus_value)
{
    return reinterpret_cast<img::auto_focus*>(focus)->analyze_frame(img,
                                                                    offsets,
                                                                    binning_value,
                                                                    *new_focus_value);
}


bool autofocus_is_running (AutoFocus* focus)
{
    return reinterpret_cast<img::auto_focus*>(focus)->is_running();
}


void autofocus_end (AutoFocus* focus)
{
    reinterpret_cast<img::auto_focus*>(focus)->end();
}


void autofocus_update_focus (AutoFocus* focus, int new_focus_value)
{
    reinterpret_cast<img::auto_focus*>(focus)->update_focus(new_focus_value);
}
