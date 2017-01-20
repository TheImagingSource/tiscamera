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

#include "image_transform_base.h"
#include <pthread.h>
#include <ctime>


namespace {
struct RegionInfo;
}

namespace img
{
class auto_focus
{
public:
    auto_focus();

    /*
     * Beware that this function currently only works for bayer-8 images and RGB32
     * @return true when a new focus value was evaluated and should be submitted to the focus control
     * @param new_focus_vale When true was returned, this contains the new focus value to set
     */
    bool analyze_frame ( const img_descriptor& img, POINT offsets, int binning_value, int& new_focus_vale );

    void run ( int focus_val, int min, int max, const RECT& roi, int speed, int auto_step_divisor, bool suggest_sweep );
    void end ();
    bool is_running () const;

    /*
     * This must be called each time when the focus value was changed for the device.
     */
    void update_focus ( int focus_val );
private:
    // this is the public interface mutex, and should never be taken by a function which is called by analyze_frame_
    // only setup/analyze_frame/run/end/update_focus/set_user_roi may take this
    // if any other function indirectly takes this lock, we run into an deadlock
    pthread_mutex_t param_mtx_;


    bool analyze_frame_ ( const img_descriptor& img, int& new_focus_vale );

    void set_focus ( int newval );

    void restart_roi ( const RegionInfo& info );
    void find_region ( const img_descriptor& image, RECT roi, RegionInfo& region );

    int calc_next_focus ();

    unsigned int get_sharpness ( const img_descriptor& img );

    bool check_wait_condition ();
    void arm_focus_timer ( int diff );

    struct data_holder
    {
        unsigned int x, y, width, height;

        unsigned int stepCount;
        int focus_val;

        int left, right;

        int prev_sharpness;
        int prev_focus;

        int sweep_step;

        enum
        {
            ended = 0,
            init,
            sweep_1,
            sweep_2,
            binary_search,
        } state;
    } data;

    RECT user_roi_;

    unsigned int init_width_, init_height_, init_pitch_;
    unsigned int init_binning_;
    POINT init_offset_;

    volatile long focus_applied_;

    int focus_min_;
    int focus_max_;
    // time in ms for moving between min and max
    int max_time_to_wait_for_focus_change_;
    // minimum speed for moving between any focus values
    int min_time_to_wait_for_focus_change_;

    int auto_step_divisor_;

    bool sweep_suggested_;

    struct timespec img_wait_endtime;
    int img_wait_cnt;

}; // class auto_focus

} // namespace img

#endif // AUTO_FOCUS_H_INC_
