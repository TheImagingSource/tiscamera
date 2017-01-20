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

#ifndef WHITEBALANCE_H
#define WHITEBALANCE_H

#include "bayer.h"


struct wb_settings
{
    bool whitebalance_is_active;
    bool auto_whitebalance;
    rgb_tripel rgb;
    rgb_tripel user_values;
    tBY8Pattern  pattern;
    unsigned int width;
    unsigned int height;
    unsigned char* data;
};


void whitebalance_buffer (struct wb_settings* settings);


#endif /* WHITEBALANCE_H */
