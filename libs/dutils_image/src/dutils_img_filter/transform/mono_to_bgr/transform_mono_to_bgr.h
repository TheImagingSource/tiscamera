
#pragma once

#include "../transform_base.h"

namespace img_filter {
namespace transform {

    // MONO8 -> RGB24/32(/64)
    transform_function_type         get_transform_mono_to_bgr_c( const img::img_type& dst, const img::img_type& src );
    transform_function_type         get_transform_mono_to_bgr_sse41( const img::img_type& dst, const img::img_type& src );
    transform_function_type         get_transform_mono_to_bgr_neon( const img::img_type& dst, const img::img_type& src );
}
}
