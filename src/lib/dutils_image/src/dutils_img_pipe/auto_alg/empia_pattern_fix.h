

#pragma once

#include <dutils_img/dutils_img.h>
#include "../../dutils_img_filter/filter/whitebalance/wb_apply.h"

namespace auto_alg::impl
{
    struct auto_sample_points;
}


namespace img_filter {
namespace empia_fix {
    img_filter::whitebalance::apply_params   calc_empia_wb_values( const img::img_descriptor& src, auto_alg::impl::auto_sample_points& scratch_space );
}
}
