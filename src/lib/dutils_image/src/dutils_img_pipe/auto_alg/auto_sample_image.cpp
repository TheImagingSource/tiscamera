
#include "auto_sample_image.h"

#include <dutils_img/pixel_structs.h>
#include <dutils_img/image_bayer_pattern.h>
#include "../../dutils_img_base/interop_private.h"

#include "image_sampling_u8.h"
#include "image_sampling_float.h"
#include "../tools/profiler_include.h"

using namespace auto_alg::impl;

bool auto_alg::impl::can_auto_sample_by_img( img::fourcc fcc ) noexcept
{
    if( img::is_byfloat_fcc( fcc ) ) {
        return true;
    }
    if( img::is_pwl_fcc( fcc ) )
    {
        return true;
    }

    return can_auto_sample_by_imgu8( fcc );
}

bool auto_alg::impl::auto_sample_by_img( const img::img_descriptor& image, auto_alg::impl::image_sampling_data& data )
{
    DUTIL_PROFILE_FUNCTION();

    assert( !image.empty() );

    if( img::is_byfloat_fcc( image.fourcc_type() ) )
    {
        auto_sample_byfloat( image, data.points_float );
        data.is_float = true;

        return data.points_float.cnt > 0;
    }
    if( img::is_pwl_fcc( image.fourcc_type() ) )
    {
        auto_sample_pwl_bayer( image, data.points_float );
        data.is_float = true;

        return data.points_float.cnt > 0;
    }
    auto_sample_by_imgu8( image, data.points_int );
    data.is_float = false;

    return data.points_float.cnt > 0;
}

auto_alg::impl::resulting_brightness	auto_alg::impl::auto_sample_mono_img( const img::img_descriptor& image )
{
    return auto_sample_mono_imgu8( image );
}

img::dim  auto_alg::impl::calc_image_sample_step_dim( const img::img_descriptor& image ) noexcept
{
    const int SAMPLING_LINES = 31;
    const int SAMPLING_COLUMNS = 41;

    if( image.dim.cx < 4 || image.dim.cy < 4 ) {    // Minimum image size is 4x4
        return img::dim{ 0, 0 };
    }

    auto steps = image.dim / img::dim{ SAMPLING_COLUMNS, SAMPLING_LINES };
    if( steps.cx == 0 ) {
        steps.cx = 1;
    }
    if( steps.cy == 0 ) {
        steps.cy = 1;
    }
    return steps;
}
