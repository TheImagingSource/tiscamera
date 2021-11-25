

#pragma once

#include "../dutils_img_base.h"
#include <dutils_img/image_transform_data_structs.h>     // img::color_matrix

namespace img_filter {
namespace transform {
namespace by_edge
{
    struct options
    {
        img::color_matrix_int	color_mtx;

        bool                use_color_matrix;
        bool                use_avg_green;
    };

    using function_type = void (*)( img::img_descriptor dst, img::img_descriptor src, const options& in_opt );

    function_type	get_transform_by8_to_dst_c( img::img_type dst, img::img_type src );
    function_type	get_transform_by8_to_dst_ssse3( img::img_type dst, img::img_type src );
    function_type	get_transform_by8_to_dst_sse41( img::img_type dst, img::img_type src );
    function_type	get_transform_by8_to_dst_avx2( img::img_type dst, img::img_type src );
    function_type	get_transform_by8_to_dst_neon( img::img_type dst, img::img_type src );
}
}
}
