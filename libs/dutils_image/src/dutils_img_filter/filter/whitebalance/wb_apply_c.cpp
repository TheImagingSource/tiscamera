
#include "wb_apply.h"

#include <algorithm>

auto    img_filter::whitebalance::get_apply_img_c( img::img_type dst ) -> img_filter::whitebalance::func_type
{
    if( img::is_by8_fcc( dst.fourcc_type() ) ) {
        return &wrap_apply_func_to_u8<detail::apply_wb_by8_c>;
    } else if( img::is_by16_fcc( dst.fourcc_type() ) ) {
        return &wrap_apply_func_to_u8<detail::apply_wb_by16_c>;
    } else if( dst.fourcc_type() == img::fourcc::RGGBFloat ) {
        return detail::apply_wb_byfloat_c;
    }
    return nullptr;
}
