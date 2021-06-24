
#pragma  once

#include <dutils_img_pipe/auto_alg_params.h>

#include "auto_sample_image.h"

namespace auto_alg::impl
{
    struct auto_hdr_gain_result 
    {
        bool    value_changed = false;
        float   hdr_gain = 0.f;
    };

    auto_hdr_gain_result    auto_hdr_gain( const auto_alg::hdr_gain_selection& params, const auto_alg::impl::image_sampling_points_rgbf& points );
}

