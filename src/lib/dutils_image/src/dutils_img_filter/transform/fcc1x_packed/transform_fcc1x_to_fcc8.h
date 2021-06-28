
#pragma once

#include "../transform_base.h"
#include <dutils_img/image_bayer_pattern.h>


namespace img_filter::transform::fcc1x_packed
{
    transform_function_param_type     get_transform_fcc1x_to_fcc8_ref( const img::img_type& dst, const img::img_type& src );
    transform_function_param_type     get_transform_fcc1x_to_fcc8_c( const img::img_type& dst, const img::img_type& src );
    transform_function_param_type     get_transform_fcc1x_to_fcc8_ssse3( const img::img_type& dst, const img::img_type& src );
    transform_function_param_type     get_transform_fcc1x_to_fcc8_neon_v0( const img::img_type& dst, const img::img_type& src );
    transform_function_param_type     get_transform_fcc1x_to_fcc8_neon_v1( const img::img_type& dst, const img::img_type& src );

    transform_function_param_type     get_transform_fcc1x_to_fcc8_neon_sep( const img::img_type& dst, const img::img_type& src );

}
