
#pragma once

#include "../dutils_img_base.h"

namespace img_filter 
{
    using transform_function_type = void(*)(img::img_descriptor dst, img::img_descriptor src);

    using transform_function_param_type = void(*)( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params );
}
