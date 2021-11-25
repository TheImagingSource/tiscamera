
#pragma  once

#include <dutils_img_pipe/auto_alg_params.h>

#include "auto_sample_image.h"
#include "image_sampling_float.h"

namespace auto_alg::impl
{
    struct  auto_whitebalance_result
    {
        bool                done;
        wb_channel_factors  result;
    };

    auto_whitebalance_result	auto_whitebalance_soft( const auto_sample_points& data, const wb_channel_factors& wb );
    auto_whitebalance_result	auto_whitebalance_cam( const auto_sample_points& data, const wb_channel_factors& wb );
    auto_whitebalance_result	auto_whitebalance_soft( const image_sampling_points_rgbf& data, const wb_channel_factors& wb );

    pixel			apply_color_matrix_c( const color_matrix_float& clr, const pixel& rgb );
    RGBf			apply_color_matrix_c( const color_matrix_float& clr, const RGBf& rgb );

    RGBf			clip_RGBf_pixel_to_range( const RGBf& rgb ) noexcept;

    inline int	    calc_brightness_from_clr_avg( int r, int g, int b ) noexcept
    {
        const constexpr int r_factor = (int)((1 << 8) * 0.299f);
        const constexpr int g_factor = (int)((1 << 8) * 0.587f);
        const constexpr int b_factor = (int)((1 << 8) * 0.114f);

        return (r * r_factor + g * g_factor + b * b_factor) >> 8;
    }

    inline int	calc_brightness_from_clr_avg( const pixel vals ) noexcept
    {
        return calc_brightness_from_clr_avg( vals.r, vals.g, vals.b );
    }
    inline float	calc_brightness_from_clr_avgf( const RGBf vals ) noexcept
    {
        return vals.r * 0.299f + vals.g * 0.587f + vals.b * 0.114f;
    }

    resulting_brightness    calc_resulting_brightness_params( const image_sampling_data& sampling_data );
    void                    apply_software_params_to_sampling_data( image_sampling_data& sampling_data, const auto_alg::color_matrix_params& clr_mtx, const auto_alg::wb_channel_factors& wb_params );
    void                    apply_software_clrmtx_to_sampling_data( image_sampling_data& sampling_data, const auto_alg::color_matrix_params& clr_mtx );
    void                    apply_software_wb_to_sampling_data( image_sampling_data& sampling_data, const auto_alg::wb_channel_factors& wb_params );
}

