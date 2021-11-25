
#include "auto_alg.h"

#include <dutils_img/pixel_structs.h>
#include <dutils_img/image_bayer_pattern.h>
#include "../../dutils_img_base/interop_private.h"
#include "../tools/profiler_include.h"

#include <cmath>

using namespace auto_alg;

using by_pattern = img::by_transform::by_pattern;


auto_alg::impl::RGBf auto_alg::impl::clip_RGBf_pixel_to_range( const RGBf& rgb ) noexcept
{
    float r = CLIP( rgb.r, 0.f, 1.f );
    float g = CLIP( rgb.g, 0.f, 1.f );
    float b = CLIP( rgb.b, 0.f, 1.f );
    return auto_alg::impl::RGBf{ r, g, b };
}


auto_alg::impl::pixel auto_alg::impl::apply_color_matrix_c( const img::color_matrix_float& clr, const auto_alg::impl::pixel& rgb )
{
    int r_tmp = std::lround(((int)rgb.r) * clr.r_rfac + ((int)rgb.g) * clr.r_gfac + ((int)rgb.b) * clr.r_bfac);
    int g_tmp = std::lround(((int)rgb.r) * clr.g_rfac + ((int)rgb.g) * clr.g_gfac + ((int)rgb.b) * clr.g_bfac);
    int b_tmp = std::lround(((int)rgb.r) * clr.b_rfac + ((int)rgb.g) * clr.b_gfac + ((int)rgb.b) * clr.b_bfac);

    uint8_t r = (uint8_t) CLIP( r_tmp, 0, 0xFF );
    uint8_t g = (uint8_t) CLIP( g_tmp, 0, 0xFF );
    uint8_t b = (uint8_t) CLIP( b_tmp, 0, 0xFF );
    return auto_alg::impl::pixel{ b, g, r };
}

auto_alg::impl::RGBf  auto_alg::impl::apply_color_matrix_c( const color_matrix_float& clr, const RGBf& rgb )
{
    float r_tmp = rgb.r * clr.r_rfac + rgb.g * clr.r_gfac + rgb.b * clr.r_bfac;
    float g_tmp = rgb.r * clr.g_rfac + rgb.g * clr.g_gfac + rgb.b * clr.g_bfac;
    float b_tmp = rgb.r * clr.b_rfac + rgb.g * clr.b_gfac + rgb.b * clr.b_bfac;

    float r = CLIP( r_tmp, 0.f, 1.f );
    float g = CLIP( g_tmp, 0.f, 1.f );
    float b = CLIP( b_tmp, 0.f, 1.f );
    return auto_alg::impl::RGBf{ r, g, b };
}


static auto_alg::impl::resulting_brightness		calc_resulting_brightness_params_( const auto_alg::impl::auto_sample_points& points )
{
    assert( points.cnt > 0 );

    int gt_240_counter = 0;
    int brightness_accu = 0;
    for( int idx = 0; idx < points.cnt; ++idx )
    {
        const int y = auto_alg::impl::calc_brightness_from_clr_avg( points.samples[idx].to_pixel() );
        if( y >= 240 ) {
            ++gt_240_counter;
        }
        brightness_accu += y;
    }

    float div = 1.f / points.cnt;

    return { brightness_accu / 255.f * div, gt_240_counter * div };
}

static auto_alg::impl::resulting_brightness	        calc_resulting_brightness_params_( const auto_alg::impl::image_sampling_points_rgbf& points )
{
    assert( points.cnt > 0 );

    int gt_240_counter = 0;
    float brightness_accu = 0;
    for( int idx = 0; idx < points.cnt; ++idx )
    {
        const float y = auto_alg::impl::calc_brightness_from_clr_avgf( points.samples[idx] );
        if( y >= 0.94f ) {    // ~ 240 / 255
            ++gt_240_counter;
        }
        brightness_accu += y;
    }

    float div = 1.f / points.cnt;

    return { brightness_accu * div, gt_240_counter * div };
}

auto_alg::impl::resulting_brightness		auto_alg::impl::calc_resulting_brightness_params( const image_sampling_data& sampling_data )
{
    DUTIL_PROFILE_FUNCTION();

    if( sampling_data.is_float )
    {
        if( sampling_data.points_float.cnt <= 0 ) {
            return auto_alg::impl::resulting_brightness::invalid();
        }

        return calc_resulting_brightness_params_( sampling_data.points_float );
    }
    else
    {
        if( sampling_data.points_int.cnt <= 0 ) {
            return auto_alg::impl::resulting_brightness::invalid();
        }

        return calc_resulting_brightness_params_( sampling_data.points_int );
    }
}

void auto_alg::impl::apply_software_params_to_sampling_data( image_sampling_data& sampling_data, const auto_alg::color_matrix_params& clr_mtx, const auto_alg::wb_channel_factors& wb_params )
{
    DUTIL_PROFILE_FUNCTION();

    bool apply_wb = wb_params.r != 1.f || wb_params.g != 1.f || wb_params.b != 1.f;
    if( !apply_wb && !clr_mtx.enabled ) {
        return;
    }

    if( sampling_data.is_float )
    {
        auto& points = sampling_data.points_float;
        for( int idx = 0; idx < points.cnt; ++idx )
        {
            auto pix = points.samples[idx];
            if( clr_mtx.enabled ) {
                pix = auto_alg::impl::apply_color_matrix_c( clr_mtx.mtx, pix );
            }
            if( apply_wb ) {
                pix = auto_alg::impl::clip_RGBf_pixel_to_range( impl::RGBf{ pix.r * wb_params.r, pix.g * wb_params.g, pix.b * wb_params.b } );
            }
            points.samples[idx] = pix;
        }
    }
    else
    {
        auto& points = sampling_data.points_int;
        for( int idx = 0; idx < points.cnt; ++idx )
        {
            auto pix = points.samples[idx].to_pixel();
            if( clr_mtx.enabled ) {
                pix = auto_alg::impl::apply_color_matrix_c( clr_mtx.mtx, pix );
            }
            if( apply_wb ) {
                pix.r = (uint8_t)CLIP( pix.r * wb_params.r, 0, 255 );
                pix.g = (uint8_t)CLIP( pix.g * wb_params.g, 0, 255 );
                pix.b = (uint8_t)CLIP( pix.b * wb_params.b, 0, 255 );
            }
            points.samples[idx] = pix;
        }
    }
}


void auto_alg::impl::apply_software_clrmtx_to_sampling_data( image_sampling_data& sampling_data, const auto_alg::color_matrix_params& clr_mtx )
{
    if( !clr_mtx.enabled ) {
        return;
    }

    DUTIL_PROFILE_FUNCTION();

    if( sampling_data.is_float )
    {
        auto& points = sampling_data.points_float;
        for( int idx = 0; idx < points.cnt; ++idx )
        {
            const auto pix = points.samples[idx];
            points.samples[idx] = auto_alg::impl::apply_color_matrix_c( clr_mtx.mtx, pix );
        }
    }
    else
    {
        auto& points = sampling_data.points_int;
        for( int idx = 0; idx < points.cnt; ++idx )
        {
            const auto pix = points.samples[idx].to_pixel();
            points.samples[idx] = auto_alg::impl::apply_color_matrix_c( clr_mtx.mtx, pix );
        }
    }
}

void auto_alg::impl::apply_software_wb_to_sampling_data( image_sampling_data& sampling_data, const auto_alg::wb_channel_factors& wb_params )
{
    if( wb_params.r == 1.f && wb_params.g == 1.f && wb_params.b == 1.f ) {
        return;
    }

    DUTIL_PROFILE_FUNCTION();

    if( sampling_data.is_float )
    {
        auto& points = sampling_data.points_float;
        for( int idx = 0; idx < points.cnt; ++idx )
        {
            const auto pix = points.samples[idx];
            points.samples[idx] = auto_alg::impl::clip_RGBf_pixel_to_range( impl::RGBf{ pix.r * wb_params.r, pix.g * wb_params.g, pix.b * wb_params.b } );
        }
    }
    else
    {
        auto& points = sampling_data.points_int;
        for( int idx = 0; idx < points.cnt; ++idx )
        {
            auto pix = points.samples[idx].to_pixel();
            pix.r = (uint8_t)CLIP( pix.r * wb_params.r, 0, 255 );
            pix.g = (uint8_t)CLIP( pix.g * wb_params.g, 0, 255 );
            pix.b = (uint8_t)CLIP( pix.b * wb_params.b, 0, 255 );
            points.samples[idx] = pix;
        }
    }
}
