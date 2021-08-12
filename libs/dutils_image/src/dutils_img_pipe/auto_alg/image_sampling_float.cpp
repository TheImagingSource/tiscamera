
#include "auto_alg.h"

#include <dutils_img/pixel_structs.h>
#include <dutils_img/image_bayer_pattern.h>
#include "../../dutils_img_base/interop_private.h"
#include "../../dutils_img_filter/transform/fcc1x_packed/fcc1x_packed_to_fcc8_internal.h"
#include "../../dutils_img_filter/transform/pwl/transform_pwl_functions.h"

using namespace auto_alg::impl;

using by_pattern = img::by_transform::by_pattern;

using img::by_transform::by_pattern_alg::next_pixel;
using img::by_transform::by_pattern_alg::next_line;

struct by_sample_entries 
{
    float v00, v01, v10, v11;
};

static auto to_auto_sample_entry( by_pattern pat, by_sample_entries e ) noexcept -> RGBf
{
    switch( pat )
    {     //								 r      g                  b
    case by_pattern::RG:    return RGBf{ e.v00, (e.v01 + e.v10) / 2, e.v11, };
    case by_pattern::GR:    return RGBf{ e.v01, (e.v00 + e.v11) / 2, e.v10, };
    case by_pattern::BG:    return RGBf{ e.v11, (e.v10 + e.v01) / 2, e.v00, };
    case by_pattern::GB:    return RGBf{ e.v10, (e.v11 + e.v00) / 2, e.v01, };
    }
    UNREACHABLE_CODE();
}

using bayer_pix_sample_func_type = by_sample_entries (*)(int x, const void* cur_line, const void* nxt_line );

static void    auto_sample_bayer_image( const img::img_descriptor& image, image_sampling_points_rgbf& points, bayer_pix_sample_func_type to_auto_sample_entry_func )
{
    const auto step_dim = auto_alg::impl::calc_image_sample_step_dim( image );
    if( step_dim.empty() ) {
        return;
    }

    const auto img_pat = img::by_transform::convert_bayer_fcc_to_pattern( image.fourcc_type() );

    int cnt = 0;	// max is 42 * 32
    for( int y = step_dim.cy; y < (image.dim.cy - 1); y += step_dim.cy )
    {
        const auto line_pat = (y & 0x1) ? next_line( img_pat ) : img_pat;

        const auto* cur_line = img::get_line_start( image, y + 0 );
        const auto* nxt_line = img::get_line_start( image, y + 1 );

        for( int x = step_dim.cx; x < (image.dim.cx - 1); x += step_dim.cx )
        {
            const auto pix_pat = (x & 0x1) ? next_pixel( line_pat ) : line_pat;

            const auto bayer_sample_data = to_auto_sample_entry_func( x, cur_line, nxt_line );

            points.samples[cnt] = to_auto_sample_entry( pix_pat, bayer_sample_data );
            ++cnt;
        }
    }
    points.cnt = cnt;
}

using to_sample_entries_func = float (*)( const void*, int );

template<to_sample_entries_func func>
by_sample_entries by10or12_to_sample_entries( int x, const void* cur_line, const void* nxt_line )
{
    return by_sample_entries{
        func( cur_line, x + 0 ),
        func( cur_line, x + 1 ),
        func( nxt_line, x + 0 ),
        func( nxt_line, x + 1 ),
    };
}

static by_sample_entries byfloat_to_sample_entries( int x, const void* cur_line, const void* nxt_line )
{
    return by_sample_entries{
        static_cast<const float*>(cur_line)[x + 0],
        static_cast<const float*>(cur_line)[x + 1],
        static_cast<const float*>(nxt_line)[x + 0],
        static_cast<const float*>(nxt_line)[x + 1],
    };
}

void auto_alg::impl::auto_sample_pwl_bayer( const img::img_descriptor& image, auto_alg::impl::image_sampling_points_rgbf& points )
{
    points.cnt = 0;

    if( image.fourcc_type() == img::fourcc::PWL_RG12_MIPI ) {
        auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<img_filter::transform::pwl::calc_pwl12_mipi_pixel_to_float> );
    } else if( image.fourcc_type() == img::fourcc::PWL_RG12 ) {
        auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<img_filter::transform::pwl::calc_pwl12_pixel_to_float> );
    } else if( image.fourcc_type() == img::fourcc::PWL_RG16H12 ) {
        auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<img_filter::transform::pwl::calc_pwl16H12_pixel_to_float> );
    }
}

void auto_alg::impl::auto_sample_byfloat( const img::img_descriptor& image, auto_alg::impl::image_sampling_points_rgbf& points )
{
    points.cnt = 0;

    auto_sample_bayer_image( image, points, &byfloat_to_sample_entries );
}
//
//using mono_sample_pixel = float( * )( const void* line_start, int x );
//
//static auto_alg::impl::resulting_brightness	        auto_sample_mono_img_( const img::img_descriptor& image, mono_sample_pixel func )
//{
//    const auto step_dim = auto_alg::impl::calc_image_sample_step_dim( image );
//    if( step_dim.empty() ) {
//        return auto_alg::impl::resulting_brightness::invalid();
//    }
//
//    int cnt = 0;
//
//    int cnt_y_gt_240 = 0;
//    float pixel_accu = 0;
//    for( int y = step_dim.cy; y < image.dim.cy; y += step_dim.cy )
//    {
//        const void* line = img::get_line_start( image, y );
//        for( int x = step_dim.cx; x < image.dim.cx; x += step_dim.cx )
//        {
//            const float val = func( line, x );
//            pixel_accu += val;
//
//            ++cnt;
//            if( val >= 0.94f ) {    // ~ 240 / 255
//                ++cnt_y_gt_240;
//            }
//        }
//    }
//
//    assert( cnt > 0 );
//
//    float div_cnt = 1.f / cnt;
//
//    return {
//        pixel_accu * div_cnt,
//        cnt_y_gt_240 * div_cnt
//    };
//}
