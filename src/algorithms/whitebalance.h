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

#include "parallel.h"
#include <memory>

using namespace tcam::algorithms;

namespace tcam
{

namespace algorithms
{

namespace whitebalance
{


struct wb_settings
{
    bool whitebalance_is_active;
    bool auto_whitebalance;
    rgb_tripel rgb;
    rgb_tripel user_values;
    tBY8Pattern  pattern;

    std::shared_ptr<parallel::parallel_state> para;
};


struct para_wb_callback : parallel::func_caller
{
    wb_settings* settings;

    void call (const tcam_image_buffer& src,
               const tcam_image_buffer& dst);

};

// void get_sampling_points (struct wb_settings* settings,
//                           struct tcam_image_buffer& buffer,
//                           auto_sample_points* points);

void whitebalance_buffer (struct wb_settings* settings, tcam_image_buffer& buffer);

} // namespace algorithms

} // namespace algorithms

} // namespace tcam

#endif /* WHITEBALANCE_H */
