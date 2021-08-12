
#include "transform_pwl_functions.h"
#include "transform_pwl_to_bayerfloat_internal.h"

#include "../fcc1x_packed/fcc1x_packed_to_fcc16_internal.h"

namespace
{
    FORCEINLINE uint8_t     map_pwl_to_fcc8( int pwl_value, const transform_pwl_internal::internal_transform_params& hdr_gain_params, float wb ) noexcept
    {
        float tmp = transform_pwl_internal::transform_pwl_to_float_single_value( pwl_value );
        return transform_pwl_internal::transform_RawFloat_to_Raw8_c( tmp * wb, hdr_gain_params);
    }
}

void    img_filter::transform::pwl::update_pwl12_to_fcc8_wb_map_data( pwl12_to_fcc8_wb_map_data& data, 
    const img::pwl_transform_params& hdr_gain_params, const img_filter::whitebalance_params& wb_params )
{
    const auto wb_param_data = img_filter::normalize( wb_params );

    if( data.calc_hdr_gain_params.hdr_gain == hdr_gain_params.hdr_gain &&
        data.calc_wb_params.apply == wb_param_data.apply &&
        data.calc_wb_params.wb_rr == wb_param_data.wb_rr &&
        data.calc_wb_params.wb_gr == wb_param_data.wb_gr &&
        data.calc_wb_params.wb_bb == wb_param_data.wb_bb &&
        data.calc_wb_params.wb_gb == wb_param_data.wb_gb )
    {
        return;
    }

    const auto pwl_mul_param = transform_pwl_internal::compute_fccfloat_to_fcc8_parameter( hdr_gain_params );

    for( int pwl_value = 0; pwl_value < 4096; ++pwl_value )
    {
        data.lut_rr[pwl_value] = map_pwl_to_fcc8( pwl_value, pwl_mul_param, wb_param_data.wb_rr );
        data.lut_gr[pwl_value] = map_pwl_to_fcc8( pwl_value, pwl_mul_param, wb_param_data.wb_gr );
        data.lut_bb[pwl_value] = map_pwl_to_fcc8( pwl_value, pwl_mul_param, wb_param_data.wb_bb );
        data.lut_gb[pwl_value] = map_pwl_to_fcc8( pwl_value, pwl_mul_param, wb_param_data.wb_gb );
    }

    data.calc_wb_params = wb_param_data;
    data.calc_hdr_gain_params = hdr_gain_params;

    return;
}
