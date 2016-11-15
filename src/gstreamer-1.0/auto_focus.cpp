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

#include "auto_focus.h"

#define max(a, b) (((a) > (b)) ? (a) : (b))

namespace {

const int REGION_SIZE = 128;
const int SWEEP_SHARPNESS_THRESHOLD = 300;

//struct RegionPos

struct RegionInfo
{
    unsigned int x;
    unsigned int y;
    unsigned int width;
    unsigned int height;

    unsigned int sharpness;
    unsigned int weighted_sharpness;
};


unsigned int abs_ (int value)
{
    int t = value >> 31;
    return t ^ (value + t);
}


unsigned int sqrt_ (unsigned int n)
{
    unsigned int a = 0;
    while( n >= (2 * a) + 1 )
    {
        n -= (2 * a++) + 1;
    }
    return a;
}


static unsigned int CalcRegionCenterDistance ( const img_descriptor& image, const RegionInfo& region )
{
    int imageCenterX = image.dim_x / 2;
    int imageCenterY = image.dim_y / 2;
    int blockCenterX = region.x + region.width / 2;
    int blockCenterY = region.y + region.height / 2;

    unsigned int dx = abs_(imageCenterX - blockCenterX);
    unsigned int dy = abs_(imageCenterY - blockCenterY);

    unsigned int ndx = (dx * 100) / image.dim_x;
    unsigned int ndy = (dy * 100) / image.dim_y;

    return sqrt_(ndx * ndx + ndy * ndy);
}


template<class TDataType>
inline TDataType* get_ptr_at ( void* region_base, int x, int y, int stride )
{
    return ((TDataType*)(((unsigned char*)region_base) + y * stride)) + x;
}


template<typename TChannelType>
static unsigned int autofocus_get_contrast_ ( const img_descriptor& image, const RegionInfo& region )
{
    const unsigned int LINE_COUNT = 7;

    void* region_start = get_ptr_at<TChannelType>( image.pData, region.x, region.y, image.pitch );
    unsigned int stride = image.pitch;

    unsigned int step_y = region.height / (LINE_COUNT + 1) + 1;
    unsigned int step_x = region.width / (LINE_COUNT + 1) + 1;

    unsigned int sharpness = 0;
    for ( unsigned int y = step_y; (y+4) < region.height; y += step_y )
    {
        unsigned int max_contrast = 0;

        for ( unsigned int x = 0; (x+16) < region.width; x += 4 )
        {
            int a = 0;
            for ( unsigned int i = 0; i < 8; ++i )
                for ( unsigned int j = 0; j < 4; ++j )
                    a += *get_ptr_at<TChannelType>( region_start, x + i, y + j, stride );

            a /= 16;

            int b = 0;
            for ( unsigned int i = 8; i < 16; ++i )
                for ( unsigned int j = 0; j< 4;++j )
                    b += *get_ptr_at<TChannelType>( region_start, x + i, y + j, stride );

            b /= 16;

            unsigned int contrast = abs_(a-b);
            max_contrast = max(contrast, max_contrast);
        }
        sharpness += max_contrast;
    }
    for ( unsigned int x = step_x; (x+4) < region.width; x += step_x )
    {
        unsigned int max_contrast = 0;

        for ( unsigned int y = 0; (y+16) < region.height; y += 4 )
        {
            int a = 0;
            for ( unsigned int i = 0; i < 8; ++i )
                for ( unsigned int j = 0; j< 4;++j )
                    a += *get_ptr_at<TChannelType>( region_start, x + j, y + i, stride );

            a /= 16;

            int b = 0;
            for ( unsigned int i = 8; i < 16; ++i )
                for ( unsigned int j = 0; j< 4;++j )
                    b += *get_ptr_at<TChannelType>( region_start, x + j, y + i, stride );

            b /= 16;

            unsigned int contrast = abs_(a-b);
            max_contrast = max(contrast, max_contrast);
        }
        sharpness += max_contrast;
    }
    return sharpness;
}


static unsigned int autofocus_get_contrast ( const img_descriptor& image, const RegionInfo& region )
{
    if ( image.type == FOURCC_Y16 )
    {
        return autofocus_get_contrast_<uint16_t>( image, region );
    }
    else
    {
        return autofocus_get_contrast_<uint8_t>( image, region );
    }
}


static void autofocus_get_all_regions_ ( const img_descriptor& image, RegionInfo* regions, unsigned int regionCount )
{
    unsigned int regions_x = image.dim_x / REGION_SIZE;
    unsigned int regions_y = image.dim_y / REGION_SIZE;

    if ( regions_x * regions_y > regionCount )
    {
        return;
    }

    unsigned int start_x = (image.dim_x - regions_x * REGION_SIZE) / 2;
    unsigned int start_y = (image.dim_y - regions_y * REGION_SIZE) / 2;

    for ( unsigned int y = 0; y < regions_y; ++y )
    {
        for ( unsigned int x = 0; x < regions_x; ++x )
        {
            RegionInfo& r = regions[y*regions_x + x];
            r.x = start_x + x * REGION_SIZE;
            r.y = start_y + y * REGION_SIZE;
            r.width = REGION_SIZE;
            r.height = REGION_SIZE;

            r.sharpness = autofocus_get_contrast( image, r );
            r.weighted_sharpness = 0;
        }
    }

    for ( unsigned int y = 0; y < regions_y; ++y )
    {
        for ( unsigned int x = 0; x < regions_x; ++x )
        {
            RegionInfo& r = regions[y*regions_x + x];

            // Boost sharpness with surrounding sharpness values
            unsigned int x0 = x > 0 ? x - 1 : x;
            unsigned int x1 = x < (regions_x - 1) ? x + 1 : x;
            unsigned int y0 = y > 0 ? y - 1 : y;
            unsigned int y1 = y < (regions_y - 1) ? y + 1 : y;

            unsigned int extra_sharpness = 0;

            for (unsigned int iy = y0; iy < y1; ++iy)
            {
                for (unsigned int ix = x0; ix < x1; ++ix)
                {
                    if (iy != 0 || ix != 0)
                    {
                        RegionInfo& ri = regions[iy * regions_x + ix];
                        extra_sharpness += ri.sharpness >> 3;
                    }
                }
            }

            // Apply elliptical quadratic weighting
            unsigned int center_distance = CalcRegionCenterDistance( image, r );

            unsigned int d = (center_distance + 60);
            r.weighted_sharpness = (unsigned int)((r.sharpness + extra_sharpness) * 10000 / (d*d));
        }
    }
}


static unsigned int calc_needed_region_count ( unsigned int img_width, unsigned int img_height, unsigned int region_size )
{
    unsigned int regions_x = img_width / region_size;
    unsigned int regions_y = img_height / region_size;

    return regions_x * regions_y;
}


static void autofocus_find_region ( const img_descriptor& image, RegionInfo& region )
{
    unsigned int region_count = calc_needed_region_count( image.dim_x, image.dim_y, REGION_SIZE );
    RegionInfo* regions = new RegionInfo[region_count];

    autofocus_get_all_regions_( image, regions, region_count );

    unsigned int best_idx = 0;
    unsigned int best_weighted_sharpness = 0;

    for ( unsigned int i = 0; i < region_count; ++i )
    {
        if ( regions[i].weighted_sharpness > best_weighted_sharpness )
        {
            best_idx = i;
            best_weighted_sharpness = regions[i].weighted_sharpness;
        }
    }

    region = regions[best_idx];

    delete[] regions;
}


static unsigned int autofocus_get_sharpness ( const img_descriptor& image, const RegionInfo& region )
{
    return autofocus_get_contrast( image, region );
}


static bool is_user_roi_valid ( const img_descriptor& image, const RECT& r )
{
    if( (r.bottom - r.top) < 64 )
        return false;
    if( (r.right - r.left) < 64 )
        return false;
    if( r.top < 0 )
        return false;
    if( r.left < 0 )
        return false;
    if( r.right > (long)image.dim_x )
        return false;
    if( r.bottom > (long)image.dim_y )
        return false;
    return true;
}

};


img::auto_focus::auto_focus ()
    : focus_applied_( 1 ), focus_min_( 0 ), focus_max_( 0 ),
      max_time_to_wait_for_focus_change_( 1000 ), img_wait_cnt( 0 )//, min_time_to_wait_for_focus_change_( 1000 )
{
    data.state = data_holder::ended;
    data.focus_val = 0;
    data.stepCount = 0;
    pthread_mutex_init(&this->param_mtx_, NULL);
}


bool img::auto_focus::is_running () const
{
    return data.state != data_holder::ended;
}


void img::auto_focus::run ( int focus_val, int min, int max, const RECT& roi,
                            int speed, int auto_step_divisor, bool suggest_sweep )
{
    int ret = pthread_mutex_trylock(&param_mtx_);
    if (ret != 0)
    {
        return;
    }

    focus_min_ = min;
    focus_max_ = max;
    max_time_to_wait_for_focus_change_ = speed;
    min_time_to_wait_for_focus_change_ = 300;
    auto_step_divisor_ = auto_step_divisor;
    sweep_suggested_ = suggest_sweep;

    data.focus_val = focus_val;

    data.state = data_holder::init;
    focus_applied_ = 1;
    data.stepCount = 0;
    user_roi_ = roi;

    pthread_mutex_unlock(&param_mtx_);
}


void img::auto_focus::end ()
{
    int ret = pthread_mutex_trylock(&param_mtx_);
    if (ret != 0)
    {
        return;
    }

    data.state = data_holder::ended;

    user_roi_.left = 0;
    user_roi_.top = 0;
    user_roi_.bottom = 0;
    user_roi_.right = 0;
    pthread_mutex_unlock(&param_mtx_);

}


void debug_out ( char* format, ... )
{
#if 0 && _DEBUG
    char buf[1024] = {};

    va_list args;
    va_start(args, format);
    vsprintf_s( buf, format, args );
    va_end(args);

    ::OutputDebugStringA( buf );
#endif
}


bool img::auto_focus::analyze_frame ( const img_descriptor& img, POINT offsets, int binning_value, int& new_focus_val )
{
    // if we can't get the lock, then just ignore this frame and retry next frame
    int ret = pthread_mutex_trylock(&param_mtx_);
    if (ret != 0)
    {
        return false;
    }
    // force this to prevent too small images
    if ( img.dim_x < REGION_SIZE || img.dim_y < REGION_SIZE )
    {
        pthread_mutex_unlock(&param_mtx_);

        return false;
    }

    if ( data.state == data_holder::ended )
    {
        pthread_mutex_unlock(&param_mtx_);

        return false;
    }

    bool rval = false;
    if ( data.state == data_holder::init )
    {
        init_width_ = img.dim_x;
        init_height_ = img.dim_y;
        init_pitch_ = img.pitch;
        init_binning_ = binning_value;
        init_offset_ = offsets;

        if ( user_roi_.bottom != 0 )
        {
            user_roi_.top = (user_roi_.top - offsets.y) / binning_value;
            user_roi_.left = (user_roi_.left - offsets.x) / binning_value;
            user_roi_.bottom = (user_roi_.bottom - offsets.y) / binning_value;
            user_roi_.right = (user_roi_.right - offsets.x) / binning_value;
        }

        RegionInfo info;
        find_region( img, user_roi_, info );
        restart_roi( info );

        if ( !sweep_suggested_ || (data.prev_sharpness > SWEEP_SHARPNESS_THRESHOLD) )
        {
            data.state = data_holder::binary_search;
            data.sweep_step = 0;
        }
        else
        {
            data.state = data_holder::sweep_1;

            if ( (data.focus_val - data.left) > (data.right - data.focus_val) )
                data.sweep_step = -(data.right - data.left) / 20;
            else
                data.sweep_step = (data.right - data.left) / 20;
        }
        data.prev_focus = data.focus_val;

        new_focus_val = calc_next_focus();
        rval = true;
    }
    else
    {
        if ( img.dim_x != init_width_ || img.dim_y != init_height_ || img.pitch != init_pitch_ || init_binning_ != (unsigned int)binning_value ||
            init_offset_.x != offsets.x || init_offset_.y != offsets.y )
        {
            // when any of the parameters change, we just quit this
            data.state = data_holder::ended;

#if _DEBUG
            char buf[100] = {};
            sprintf_s( buf, __FUNCTION__ ", preconditions changed, just quit one-push-run\n" );
            OutputDebugStringA( buf );
#endif
            pthread_mutex_unlock(&param_mtx_);

            return false;
        }
        if ( !focus_applied_ )
        {
            pthread_mutex_unlock(&param_mtx_);

            return false;
        }
        if ( check_wait_condition() )
        {
            rval = analyze_frame_( img, new_focus_val );
        }
    }

    if ( rval )
    {
        focus_applied_ = 0;
        arm_focus_timer( abs_(data.prev_focus - new_focus_val) );
    }
    pthread_mutex_unlock(&param_mtx_);

    return rval;
}


bool img::auto_focus::analyze_frame_ ( const img_descriptor& img, int& new_focus_val )
{
    data.stepCount += 1;
    if ( (data.stepCount == 4 || data.stepCount == 8) )
    {
        RegionInfo newROI;
        find_region( img, user_roi_, newROI );
        if ( data.x != newROI.x || data.y != newROI.y )
        {
            restart_roi( newROI );
            new_focus_val = calc_next_focus();
            return true;
        }
    }

    // In sweep mode, look for the region with the highest sharpness on every frame
    if ( (data.state == data_holder::sweep_1) || (data.state == data_holder::sweep_2) )
    {
        RegionInfo newROI;
        find_region(img, user_roi_, newROI);
        data.x = newROI.x;
        data.y = newROI.y;
        data.width = newROI.width;
        data.height = newROI.height;
    }

    int sq = get_sharpness( img );

    if ( (data.state == data_holder::sweep_1) || (data.state == data_holder::sweep_2) )
    {
        if ( sq > SWEEP_SHARPNESS_THRESHOLD )
        {
            data.state = data_holder::binary_search;

            if ( data.sweep_step > 0 )
                data.left = data.focus_val - data.sweep_step;
            else
                data.right = data.focus_val - data.sweep_step;

            data.prev_focus = data.focus_val;
            data.prev_sharpness = sq;
            new_focus_val = calc_next_focus();

            // debug_out( "SWEEP SUCCESS: focus_val = %d, sharpness = %d => state = binary_search, new_focus_val = %d\n", data.focus_val, sq, new_focus_val );

            return true;
        }
        else
        {
            new_focus_val = data.focus_val + data.sweep_step;
            if ( new_focus_val < data.left ) new_focus_val = data.left;
            if ( new_focus_val > data.right) new_focus_val = data.right;

            if ( ((data.sweep_step < 0) && (new_focus_val == data.left))
                || ((data.sweep_step > 0) && (new_focus_val == data.right)) )
            {
                if ( data.state == data_holder::sweep_1 )
                {
                    data.state = data_holder::sweep_2;
                    data.sweep_step = -data.sweep_step;

                    // Restart from where sweep_1 was started
                    new_focus_val = data.prev_focus;
                }
                else
                {
                    data.state = data_holder::binary_search;

                    data.prev_focus = data.focus_val;
                    data.prev_sharpness = sq;
                    new_focus_val = calc_next_focus();
                }
            }

            return true;
        }
    }

    if ( data.state == data_holder::binary_search )
    {
        if ( sq < data.prev_sharpness )
        {
            if ( data.focus_val < data.prev_focus )
                data.left = data.focus_val;
            else
                data.right = data.focus_val;
        }
        else // sq >= prev_sharpness
        {
            if ( data.focus_val < data.prev_focus )
                data.right = data.prev_focus;
            else
                data.left = data.prev_focus;

            data.prev_focus = data.focus_val;
            data.prev_sharpness = sq;
        }

        if ( (data.right - data.left) > 2 )
        {
            new_focus_val = calc_next_focus();

            return true;
        }
        else
        {
            data.state = data_holder::ended;
        }
    }

    return false;
}


unsigned int img::auto_focus::get_sharpness ( const img_descriptor& img )
{
    RegionInfo info;
    info.x = data.x;
    info.y = data.y;
    info.width = data.width;
    info.height = data.height;

    return autofocus_get_sharpness( img, info );
}


int img::auto_focus::calc_next_focus ()
{
    unsigned int dl = abs_(data.left - data.prev_focus);
    unsigned int dr = abs_(data.right - data.prev_focus);

    int div = auto_step_divisor_;
    unsigned int lstep = dl / div;
    unsigned int rstep = dr / div;
    if ( !lstep ) lstep = 1;
    if ( !rstep ) rstep = 1;

    int new_focus = (dl > dr) ? (data.prev_focus - lstep) : (data.prev_focus + rstep);

    return new_focus;
}


void img::auto_focus::restart_roi ( const RegionInfo& info )
{
    data.x = info.x;
    data.y = info.y;
    data.width = info.width;
    data.height = info.height;
    data.left = focus_min_;
    data.right = focus_max_;

    data.prev_sharpness = info.sharpness;
    data.prev_focus = data.focus_val;
}


void img::auto_focus::update_focus ( int focus_val )
{
    int ret = pthread_mutex_trylock(&param_mtx_);

    if (ret != 0)
        return;

    data.focus_val = focus_val;
    focus_applied_ = 1;

    pthread_mutex_unlock(&param_mtx_);
}


bool img::auto_focus::check_wait_condition ()
{
    if ( --img_wait_cnt < 0 )
    {
        timespec time2;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        if (time2.tv_sec > img_wait_endtime.tv_sec)
        {
            return true;
        }

        if (time2.tv_sec == img_wait_endtime.tv_sec)
            if (time2.tv_nsec > img_wait_endtime.tv_nsec)
            {
                return true;
            }
    }
    return false;
}


void img::auto_focus::arm_focus_timer ( int diff )
{
    int ms_to_use = 0;
    if( diff > 0 )
    {
        ms_to_use = (diff * max_time_to_wait_for_focus_change_) / (focus_max_ - focus_min_);
    }
    // we have to wait for at least n frames and at least for focus_min_move_speed_ milliseconds
    if( ms_to_use < min_time_to_wait_for_focus_change_ )
        ms_to_use = min_time_to_wait_for_focus_change_;

    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &img_wait_endtime);
    img_wait_endtime.tv_nsec = ms_to_use*1000*1000;

    img_wait_cnt = 3; // at least we have to wait 2 frames due to frame latency
}


void img::auto_focus::find_region ( const img_descriptor& image, RECT roi, RegionInfo& region )
{
    if ( is_user_roi_valid( image, roi ) )
    {
        RegionInfo& tmp = region;
        tmp.width = roi.right - roi.left;
        tmp.height = roi.bottom - roi.top;
        tmp.x = roi.left;
        tmp.y = roi.top;
        region.sharpness = autofocus_get_sharpness( image, tmp );

        user_roi_ = roi;
    }
    else
    {
        RECT r = {};
        user_roi_ = r;
        autofocus_find_region( image, region );
    }
}
