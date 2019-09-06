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

#ifndef AUTO_FOCUS_H_INC_
#define AUTO_FOCUS_H_INC_

#pragma once

// #include <dutils_img_pipe/auto_alg_params.h>

#include "image_transform_base.h"
#include "auto_alg_params.h"

namespace
{
struct RegionInfo;
}

namespace auto_alg
{

class auto_focus
{
public:
    auto_focus ();

    bool auto_alg_run (uint64_t time_point,
                       const img::img_descriptor& img,
                       const auto_alg::auto_focus_params& state,
                       img::point offsets,
                       img::dim pixel_dim,
                       int& new_focus_vale);

    bool is_running () const;

    void reset () { end(); }

private:
    /*
     * Beware that this function currently only works for bayer-8 images and RGB32
     *	@return	true when a new focus value was evaluated and should be submitted to the focus control
     *	@param new_focus_vale	When true was returned, this contains the new focus value to set
     */
    bool	analyze_frame (uint64_t now, const img::img_descriptor& img, int& new_focus_vale);

    void	run (int focus_val,
                 int min, int max,
                 const img::rect& roi,
                 int speed, int auto_step_divisor,
                 bool suggest_sweep);
    void	end ();
    bool	analyze_frame_ (const img::img_descriptor& img, int& new_focus_vale);

    void	restart_roi (const RegionInfo& info);
    void	find_region (const img::img_descriptor& image, img::rect roi, RegionInfo& region);

    int		calc_next_focus ();

    int		get_sharpness (const img::img_descriptor& img);

    bool	check_wait_condition (uint64_t now);
    void	arm_focus_timer (uint64_t now, int diff);

    void	update_focus (int focus_val);

    struct data_holder
    {
        int	x, y, width, height;

        int	    stepCount;
        int		focus_val;

        int		left, right;

        int		prev_sharpness;
        int		prev_focus;

        int		sweep_step;

        enum
        {
            ended = 0,
            init,
            sweep_1,
            sweep_2,
            binary_search,
        } state;
    } data;

    img::rect			user_roi_;

    int	            init_width_, init_height_;
    int             init_pitch_;
    img::dim       init_pixel_dim_;
    img::point		init_offset_;

    int				focus_min_;
    int				focus_max_;
    // time in ms for moving between min and max
    int				max_time_to_wait_for_focus_change_;
    // minimum speed for moving between any focus values
    int				min_time_to_wait_for_focus_change_;

    int				auto_step_divisor_;

    bool			sweep_suggested_;

    uint64_t        img_wait_endtime_;  // in us
    int				img_wait_cnt_;
};

} /* namespace auto_alg */

#endif // AUTO_FOCUS_H_INC_
