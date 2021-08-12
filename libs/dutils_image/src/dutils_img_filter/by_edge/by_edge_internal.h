
#pragma once

#include "by_edge.h"

#include <dutils_img/pixel_structs.h>
#include <dutils_img/image_bayer_pattern.h>

namespace by_edge_internal
{
    using img::pixel_type::BGRA32;
    using img::pixel_type::BGR24;

    using namespace img::by_transform;

    using alg_context_c = img_filter::transform::by_edge::options;

    struct line_data
    {
        uint8_t*   lines[3];    // { prv, cur, nxt }

        void*   out_line;       // pointer to the out line, must be converted to the actual type
    };

    inline line_data init_src_param( int y, const img::img_descriptor& dst, const img::img_descriptor& src, int offset_prev, int offset_next ) noexcept
    {
        line_data line0 = { {
                img::get_line_start( src, (y + offset_prev) ),
                img::get_line_start( src, (y + 0) ),
                img::get_line_start( src, (y + offset_next) ),
            },
            img::get_line_start( dst, y ),
        };
        return line0;
    }

    constexpr bool  is_red_line( by_pattern pat ) noexcept {
        return pat == by_pattern::RG || pat == by_pattern::GR;
    }
    constexpr bool  is_gx_line( by_pattern pat ) noexcept  {
        return pat == by_pattern::GR || pat == by_pattern::GB;
    }

    template<class TOut>    int	    conv_by8_line_c( by_pattern pattern, const alg_context_c& clr, const line_data& lines, int x, int dim_x );
    template<>              int	    conv_by8_line_c<BGRA32>( by_pattern pattern, const alg_context_c& clr, const line_data& lines, int x, int dim_x );
    template<>              int	    conv_by8_line_c<BGR24>( by_pattern pattern, const alg_context_c& clr, const line_data& lines, int x, int dim_x );
}