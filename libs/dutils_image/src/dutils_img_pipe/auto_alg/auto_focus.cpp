

#include "auto_focus.h"


#include <algorithm>

#include "../../dutils_img_base/img_rect_tools.h"

namespace
{
    using std::max;

const int REGION_SIZE = 128;
const int SWEEP_SHARPNESS_THRESHOLD = 300;

struct RegionInfo
{
    int x;
    int y;
    int width;
    int height;

    int sharpness;
    int weighted_sharpness;
};

int abs_(int value)
{
    int t = value >> 31; 
    return t ^ (value + t);
}

int sqrt_(int n){
    if( n < 0 ) return 0;
    int a = 0;
    while( n >= (2 * a) + 1 )
    {
        n -= (2 * a++) + 1;
    }
    return a;
}


static int		CalcRegionCenterDistance( const img::img_descriptor& image, const RegionInfo& region )
{
    int imageCenterX = image.dim.cx / 2;
    int imageCenterY = image.dim.cy / 2;
    int blockCenterX = region.x + region.width / 2;
    int blockCenterY = region.y + region.height / 2;

    int dx = abs_(imageCenterX - blockCenterX);
    int dy = abs_(imageCenterY - blockCenterY);

    int ndx = (dx * 100) / image.dim.cx;
    int ndy = (dy * 100) / image.dim.cy;

    return sqrt_(ndx * ndx + ndy * ndy);
}

template<class TDataType>
inline TDataType*	get_ptr_at( void* region_base, int x, int y, int stride )
{
    return ((TDataType*)(((uint8_t*)region_base) + y * stride)) + x;
}

template<typename TChannelType>
static int autofocus_get_contrast_( const img::img_descriptor& image, const RegionInfo& region )
{
    const int LINE_COUNT = 7;

    void* region_start = img::get_line_start<TChannelType>( image, region.y ) + region.x;
    int stride = image.pitch();

    int step_y = region.height / (LINE_COUNT + 1) + 1;
    int step_x = region.width / (LINE_COUNT + 1) + 1;

    int sharpness = 0;
    for( int y = step_y; (y+4) < region.height; y += step_y )
    {
        int max_contrast = 0;

        for( int x = 0; (x+16) < region.width; x += 4 )
        {
            int a = 0;
            for( int i = 0; i < 8; ++i )
                for( int j = 0; j < 4; ++j )
                    a += *get_ptr_at<TChannelType>( region_start, x + i, y + j, stride );

            a /= 16;

            int b = 0;
            for( int i = 8; i < 16; ++i )
                for( int j = 0; j< 4;++j )
                    b += *get_ptr_at<TChannelType>( region_start, x + i, y + j, stride );

            b /= 16;

            int contrast = abs_(a-b);
            max_contrast = max(contrast, max_contrast);
        }
        sharpness += max_contrast;
    }
    for( int x = step_x; (x+4) < region.width; x += step_x )
    {
        int max_contrast = 0;

        for( int y = 0; (y+16) < region.height; y += 4 )
        {
            int a = 0;
            for( int i = 0; i < 8; ++i )
                for( int j = 0; j< 4;++j )
                    a += *get_ptr_at<TChannelType>( region_start, x + j, y + i, stride );

            a /= 16;

            int b = 0;
            for( int i = 8; i < 16; ++i )
                for( int j = 0; j< 4;++j )
                    b += *get_ptr_at<TChannelType>( region_start, x + j, y + i, stride );

            b /= 16;

            int contrast = abs_(a-b);
            max_contrast = max(contrast, max_contrast);
        }
        sharpness += max_contrast;
    }
    return sharpness;
}

static int autofocus_get_contrast( const img::img_descriptor& image, const RegionInfo& region )
{
    if( image.fourcc_type() == img::fourcc::MONO16 || img::is_by16_fcc( image.fourcc_type() ) )
    {
        return autofocus_get_contrast_<uint16_t>( image, region );
    }
    else
    {
        return autofocus_get_contrast_<uint8_t>( image, region );
    }
}

static void autofocus_get_all_regions_( const img::img_descriptor& image, RegionInfo* regions, int regionCount )
{
    int regions_x = image.dim.cx / REGION_SIZE;
    int regions_y = image.dim.cy / REGION_SIZE;

    if( regions_x * regions_y > regionCount )
    {
        return;
    }

    int start_x = (image.dim.cx - regions_x * REGION_SIZE) / 2;
    int start_y = (image.dim.cy - regions_y * REGION_SIZE) / 2;

    for( int y = 0; y < regions_y; ++y )
    {
        for( int x = 0; x < regions_x; ++x )
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

    for( int y = 0; y < regions_y; ++y )
    {
        for( int x = 0; x < regions_x; ++x )
        {
            RegionInfo& r = regions[y*regions_x + x];

            // Boost sharpness with surrounding sharpness values
            int x0 = x > 0 ? x - 1 : x;
            int x1 = x < (regions_x - 1) ? x + 1 : x;
            int y0 = y > 0 ? y - 1 : y;
            int y1 = y < (regions_y - 1) ? y + 1 : y;

            int extra_sharpness = 0;

            for (int iy = y0; iy < y1; ++iy)
            {
                for (int ix = x0; ix < x1; ++ix)
                {
                    if (iy != 0 || ix != 0)
                    {
                        RegionInfo& ri = regions[iy * regions_x + ix];
                        extra_sharpness += ri.sharpness >> 3;
                    }
                }
            }

            // Apply elliptical quadratic weighting
            int center_distance = CalcRegionCenterDistance( image, r ); 

            int d = (center_distance + 60);
            r.weighted_sharpness = (r.sharpness + extra_sharpness) * 10000 / (d*d);
        }
    }
}

static int	calc_needed_region_count( int img_width, int img_height, int region_size )
{
    int regions_x = img_width / region_size;
    int regions_y = img_height / region_size;

    return regions_x * regions_y;
}

static void	autofocus_find_region( const img::img_descriptor& image, RegionInfo& region )
{
    int region_count = calc_needed_region_count( image.dim.cx, image.dim.cy, REGION_SIZE );
    RegionInfo* regions = new RegionInfo[region_count];

    autofocus_get_all_regions_( image, regions, region_count );

    int best_idx = 0;
    int best_weighted_sharpness = 0;

    for( int i = 0; i < region_count; ++i )
    {
        if( regions[i].weighted_sharpness > best_weighted_sharpness )
        {
            best_idx = i;
            best_weighted_sharpness = regions[i].weighted_sharpness;
        }
    }	

    region = regions[best_idx];

    delete[] regions;
}

static int autofocus_get_sharpness( const img::img_descriptor& image, const RegionInfo& region )
{
    return autofocus_get_contrast( image, region );
}

static bool is_user_roi_valid( const img::img_descriptor& image, const img::rect& r )
{
    if( (r.bottom - r.top) < 64 )
        return false;
    if( (r.right - r.left) < 64 )
        return false;
    if( r.top < 0 )
        return false;
    if( r.left < 0 )
        return false;
    if( r.right > image.dim.cx )
        return false;
    if( r.bottom > image.dim.cy )
        return false;
    return true;
}

}

auto_alg::impl::auto_focus::auto_focus()
    : focus_min_( 0 ), focus_max_( 0 ), 
    max_time_to_wait_for_focus_change_( 1000 ), min_time_to_wait_for_focus_change_( 300 )
{
    data.state = data_holder::ended;
    data.focus_val = 0;
    data.stepCount = 0;
}

bool auto_alg::impl::auto_focus::is_running() const
{
    return data.state != data_holder::ended;
}

void auto_alg::impl::auto_focus::setup_run( int focus_val, const auto_alg::auto_focus_params::run_cmd_param_struct& params, img::rect roi )
{
    focus_min_ = params.focus_range_min;
    focus_max_ = params.focus_range_max;
    max_time_to_wait_for_focus_change_ = params.focus_device_speed;
    min_time_to_wait_for_focus_change_ = 300;
    auto_step_divisor_ = params.auto_step_divisor;
    sweep_suggested_ = params.suggest_sweep;

    data.focus_val = focus_val;

    data.state = data_holder::init;
    data.stepCount = 0;
    user_roi_ = roi;
}

void auto_alg::impl::auto_focus::end()
{
    data.state = data_holder::ended;

    user_roi_ = {};
}


bool auto_alg::impl::supports_auto_focus( const img::img_type& img ) noexcept
{
    // these formats should work ...
    if( img.fourcc_type() != img::fourcc::MONO8 &&
        img.fourcc_type() != img::fourcc::MONO16 &&
        img::is_by16_fcc( img.fourcc_type() ) &&
        img::is_by8_fcc( img.fourcc_type() ) &&
        img.fourcc_type() != img::fourcc::BGRA32 &&
        img.fourcc_type() != img::fourcc::BGR24 )
    {
        return false;
    }

    // force this to prevent too small images
    if( img.dim.cx < REGION_SIZE || img.dim.cy < REGION_SIZE ) {
        return false;
    }
    return true;
}


bool auto_alg::impl::auto_focus::is_auto_alg_run_needed( const auto_alg::auto_focus_params& params ) const noexcept
{
    if( !params.enable_focus ) {
        return false;
    }
    if( params.is_end_cmd || params.is_run_cmd ) {
        return true;
    }
    if( data.state == data_holder::ended ) {
        return false;
    }
    return true;
}



bool auto_alg::impl::auto_focus::auto_alg_run( uint64_t time_point, const img::img_descriptor& img, const auto_alg::auto_focus_params& state, 
    img::point offsets, img::dim pixel_dim, int& new_focus_val )
{
    if( !supports_auto_focus( img.to_img_type() ) ) {
        return false;
    }

    if( state.is_end_cmd )
    {
        end();
        return false;
    }

    if( state.is_run_cmd )
    {
        auto roi = img::clip_to_img_desc_region( state.run_cmd_params.roi, offsets, pixel_dim, img );
        setup_run( state.device_focus_val, state.run_cmd_params, roi );

        init_dim_ = img.dim;
        init_offset_ = offsets;

        return analyze_frame( time_point, img, new_focus_val );
    }
    else 
    {
        if( data.state == data_holder::ended ) {
            return false;
        }

        if( img.dim != init_dim_ || init_offset_ != offsets )
        {
            // when any of the parameters change, we just quit this
            end();

            return false;
        }

        update_focus( state.device_focus_val );

        return analyze_frame( time_point, img, new_focus_val );
    }
}


#if 1
#define debug_out( ... )
#else

#include <stdio.h>
#include <stdarg.h>

extern "C" {
__declspec(dllimport)
void
OutputDebugStringA(
    const char* lpOutputString
    );
}

void	debug_out( char* format, ... )
{
    char buf[1024] = {};

    va_list args;
    va_start(args, format);	
    vsprintf_s( buf, format, args );
    va_end(args);

    ::OutputDebugStringA( buf );
}
#endif

#include <cstdio>

bool	auto_alg::impl::auto_focus::analyze_frame( uint64_t now, const img::img_descriptor& img, int& new_focus_val )
{
    bool rearm_timer = false;
    if( data.state == data_holder::init )
    {
        RegionInfo info;
        find_region( img, user_roi_, info );
        restart_roi( info );

        if( !sweep_suggested_ || (data.prev_sharpness > SWEEP_SHARPNESS_THRESHOLD) )
        {
            data.state = data_holder::binary_search;
            data.sweep_step = 0;

            debug_out( "INIT: focus = %d, sharpness = %d => state = binary_search\n", data.focus_val, data.prev_sharpness );
        }
        else
        {
            data.state = data_holder::sweep_1;

            if( (data.focus_val - data.left) > (data.right - data.focus_val) )
                data.sweep_step = -(data.right - data.left) / 20;
            else
                data.sweep_step = (data.right - data.left) / 20;

            debug_out( "INIT: focus = %d, sharpness = %d => state = sweep_1, step = %d\n", data.focus_val, data.prev_sharpness, data.sweep_step );
        }
        data.prev_focus = data.focus_val;

        new_focus_val = calc_next_focus();
        rearm_timer = true;
    }
    else
    {
        if( check_wait_condition( now ) )
        {
            //printf( "data.state != data_holder::init && check_wait_condition == true\n" );
            rearm_timer = analyze_frame_( img, new_focus_val );
        }
        else
        {
            //printf( "data.state != data_holder::init && check_wait_condition != true\n" );
        }
    }

    if( rearm_timer )
    {
        arm_focus_timer( now, abs_(data.prev_focus - new_focus_val) );
    }

    return rearm_timer;
}

bool	auto_alg::impl::auto_focus::analyze_frame_( const img::img_descriptor& img, int& new_focus_val )
{
    data.stepCount += 1;
    if( (data.stepCount == 4 || data.stepCount == 8) )
    {
        RegionInfo	newROI;
        find_region( img, user_roi_, newROI );
        if( data.x != newROI.x || data.y != newROI.y )
        {
            restart_roi( newROI );
            new_focus_val = calc_next_focus();
            return true;
        }
    }

    // In sweep mode, look for the region with the highest sharpness on every frame
    if( (data.state == data_holder::sweep_1) || (data.state == data_holder::sweep_2) )
    {
        RegionInfo newROI;
        find_region(img, user_roi_, newROI);
        data.x = newROI.x;
        data.y = newROI.y;
        data.width = newROI.width;
        data.height = newROI.height;
    }

    int sq = get_sharpness( img );

    if( (data.state == data_holder::sweep_1) || (data.state == data_holder::sweep_2) )
    {
        if( sq > SWEEP_SHARPNESS_THRESHOLD )
        {
            data.state = data_holder::binary_search;

            if( data.sweep_step > 0 )
                data.left = data.focus_val - data.sweep_step;
            else
                data.right = data.focus_val - data.sweep_step;

            data.prev_focus = data.focus_val;
            data.prev_sharpness = sq;
            new_focus_val = calc_next_focus();

            debug_out( "SWEEP SUCCESS: focus_val = %d, sharpness = %d => state = binary_search, new_focus_val = %d\n", data.focus_val, sq, new_focus_val );

            return true;
        }
        else
        {
            new_focus_val = data.focus_val + data.sweep_step;
            if( new_focus_val < data.left ) new_focus_val = data.left;
            if( new_focus_val > data.right) new_focus_val = data.right;

            if( ((data.sweep_step < 0) && (new_focus_val == data.left))
                || ((data.sweep_step > 0) && (new_focus_val == data.right)) )
            {
                if( data.state == data_holder::sweep_1 )
                {
                    data.state = data_holder::sweep_2;
                    data.sweep_step = -data.sweep_step;

                    // Restart from where sweep_1 was started
                    new_focus_val = data.prev_focus;

                    debug_out( "SWEEP REVERSE: focus_val = %d, sharpness = %d, sweep_step = %d => state = sweep_2, new_focus_val = %d\n", data.focus_val, sq, data.sweep_step, new_focus_val );
                }
                else
                {
                     data.state = data_holder::binary_search;

                     data.prev_focus = data.focus_val;
                     data.prev_sharpness = sq;
                     new_focus_val = calc_next_focus();

                     debug_out( "SWEEP FAIL: focus_val = %d, sharpness = %d => state = binary_search, new_focus_val = %d\n", data.focus_val, sq, new_focus_val );
                }
            }
            else
            {
                debug_out( "SWEEP STEP: focus_val = %d, sharpness = %d, sweep_step = %d => new_focus_val = %d\n", data.focus_val, sq, data.sweep_step, new_focus_val );				
            }			

            return true;
        }
    }
    
    if( data.state == data_holder::binary_search )
    {
        if( sq < data.prev_sharpness )
        {
            if( data.focus_val < data.prev_focus )
                data.left = data.focus_val;
            else
                data.right = data.focus_val;
        }
        else // sq >= prev_sharpness
        {
            if( data.focus_val < data.prev_focus )
                data.right = data.prev_focus;
            else
                data.left = data.prev_focus;

            data.prev_focus = data.focus_val;
            data.prev_sharpness = sq;
        }

        if( (data.right - data.left) > 2 )
        {
            new_focus_val = calc_next_focus();

            debug_out( "BINARY_SEARCH: cur_focus=%d, next_focus=%d, left=%d, right=%d, cur_sharpness=%d region = (%d, %d)\n",
                data.focus_val, new_focus_val, data.left, data.right, sq,
                data.x, data.y
                );

            return true;
        }
        else
        {
            data.state = data_holder::ended;
        }
    }

    return false;
}

int auto_alg::impl::auto_focus::get_sharpness( const img::img_descriptor& img )
{
    RegionInfo	info = {};
    info.x = data.x;
    info.y = data.y;
    info.width = data.width;
    info.height = data.height;

    return autofocus_get_sharpness( img, info );
}

int auto_alg::impl::auto_focus::calc_next_focus()
{
    int dl = abs_(data.left - data.prev_focus);
    int dr = abs_(data.right - data.prev_focus);

    int div = auto_step_divisor_;
    int lstep = dl / div;
    int rstep = dr / div;
    if( !lstep ) lstep = 1;
    if( !rstep ) rstep = 1;
    
    int new_focus = (dl > dr) ? (data.prev_focus - lstep) : (data.prev_focus + rstep);

    return new_focus;
}

void auto_alg::impl::auto_focus::restart_roi( const RegionInfo& info )
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

void auto_alg::impl::auto_focus::update_focus( int focus_val )
{
    data.focus_val = focus_val;
}

bool auto_alg::impl::auto_focus::check_wait_condition( uint64_t now )
{
    if( img_wait_cnt_ > 0 )
    {
        --img_wait_cnt_;
        return false;
    }
    img_wait_cnt_ = 0;
    //printf( "now=%llu, img_wait_endtime_=%llu\n", now, img_wait_endtime_ );
    return now > img_wait_endtime_;
}

void auto_alg::impl::auto_focus::arm_focus_timer( uint64_t now, int diff )
{
    int ms_to_use = 0;
    if( diff > 0 )
    {
        ms_to_use = (diff * max_time_to_wait_for_focus_change_) / (focus_max_ - focus_min_);
    }
    // we have to wait for at least n frames and at least for focus_min_move_speed_ milliseconds 
    if( ms_to_use < min_time_to_wait_for_focus_change_ ) {
        ms_to_use = min_time_to_wait_for_focus_change_;
    }

    img_wait_endtime_ = now + ms_to_use * 1000;

    img_wait_cnt_ = 3;	// at least we have to wait 2 frames due to frame latency
}

void auto_alg::impl::auto_focus::find_region( const img::img_descriptor& image, img::rect roi, RegionInfo& region )
{
    if( is_user_roi_valid( image, roi ) )
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
        user_roi_ = {};
        autofocus_find_region( image, region );

        debug_out( "FIND REGION: x = %d, y = %d\n", region.x, region.y );
    }
}
