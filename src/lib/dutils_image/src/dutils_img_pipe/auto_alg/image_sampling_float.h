
#pragma  once

#include <dutils_img_pipe/auto_alg_params.h>

#include <dutils_img/pixel_structs.h>
#include "auto_sample_image.h"

namespace auto_alg::impl
{
    // Sampling functions
    void                    auto_sample_byfloat( const img::img_descriptor& image, image_sampling_points_rgbf& points );
    void                    auto_sample_pwl_bayer( const img::img_descriptor& image, image_sampling_points_rgbf& points );
}

