
#include "wb_apply.h"

auto    img_filter::whitebalance::get_apply_img_neon( img::img_type dst ) -> img_filter::whitebalance::func_type
{
    if( dst.dim.cx < 16 ) {
        return nullptr;
    }

    if( img::is_by8_fcc( dst.fourcc_type() ) ) {
        return wrap_apply_func_to_u8<&detail::apply_wb_by8_neon>;
    } else if( img::is_by16_fcc( dst.fourcc_type() ) ) {
        return wrap_apply_func_to_u8<&detail::apply_wb_by16_neon>;
    }
    return nullptr;
}
