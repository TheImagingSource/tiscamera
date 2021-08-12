
#include "auto_hdr_gain.h"

#include "auto_alg.h"

#include "../../dutils_img_filter/transform/pwl/transform_pwl_to_bayerfloat_internal.h"

#include <cmath>    // std::log
#include <algorithm>

// #TODO add ExposureAutoReference as parameter
auto_alg::impl::auto_hdr_gain_result auto_alg::impl::auto_hdr_gain( const auto_alg::hdr_gain_selection& params, 
    const auto_alg::impl::image_sampling_points_rgbf& points )
{
    assert( points.cnt > 0 );
    assert( params.enable_auto_hdr_gain_selection );

    const auto prev_value = std::clamp( params.transform_param.hdr_gain, 0.f, 120.f ); // clip to range to prevent values out of range

    const auto internal_params = transform_pwl_internal::compute_fccfloat_to_fcc8_parameter( params.transform_param );
    int sum_of_covnerted_gray_values = 0;

    float reference_value = params.hdr_gain_auto_reference * 256.f;

    for( int i = 0; i < points.cnt; ++i )
    {
        // first convert to gray
        const float gray_value = auto_alg::impl::calc_brightness_from_clr_avgf( points.samples[i] );
        const uint8_t converted_gray_value = transform_pwl_internal::transform_RawFloat_to_Raw8_c( gray_value, internal_params );
        sum_of_covnerted_gray_values += converted_gray_value;
    }

    float new_hdrgain_value = 0.f;
    if( sum_of_covnerted_gray_values <= 0 )
    {
        new_hdrgain_value = prev_value + 48.0f; // increase gain by a lot if the image is entirely black
    }
    else
    {
        const auto avg_gray_value = (float)sum_of_covnerted_gray_values / (float)points.cnt;
        const auto desired_factor = reference_value / avg_gray_value;
        const auto delta_gain = std::log10(desired_factor) * 20.0f;
        new_hdrgain_value = prev_value + delta_gain;
    }

    new_hdrgain_value = std::clamp( new_hdrgain_value, 0.f, 120.f );   // clamp to actual range to prevent unrestricted growth

    return auto_alg::impl::auto_hdr_gain_result{ new_hdrgain_value != prev_value, new_hdrgain_value };
}
