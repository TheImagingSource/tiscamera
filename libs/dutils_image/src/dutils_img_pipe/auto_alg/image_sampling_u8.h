
#pragma  once

#include <dutils_img_pipe/auto_alg_params.h>

#include <dutils_img/pixel_structs.h>
#include "auto_sample_image.h"

namespace auto_alg::impl
{
    // Sampling functions
    void                    auto_sample_by_imgu8( const img::img_descriptor& image, auto_sample_points& points );
    resulting_brightness    auto_sample_mono_imgu8( const img::img_descriptor& image );

    bool                    can_auto_sample_by_imgu8( img::fourcc fcc );
}

