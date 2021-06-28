
#pragma once

#include "../transform_base.h"
#include <dutils_img/image_transform_data_structs.h>

namespace img_filter
{
    struct pwl12_to_fcc8_wb_map_data
    {
        img_filter::whitebalance_params calc_wb_params{ true, -1.f };   // force to something invalid
        img::pwl_transform_params       calc_hdr_gain_params;

        uint8_t lut_rr[4096];
        uint8_t lut_gr[4096];
        uint8_t lut_bb[4096];
        uint8_t lut_gb[4096];
    };

namespace transform{
namespace pwl
{
    using transform_func_type = void(*)(img::img_descriptor dst, img::img_descriptor src, const img::pwl_transform_params& params );

    transform_function_type      get_transform_pwl_to_fccfloat_ref( const img::img_type& dst, const img::img_type& src );
    transform_function_type      get_transform_pwl_to_fccfloat_c( const img::img_type& dst, const img::img_type& src );
    transform_function_type      get_transform_pwl_to_fccfloat_c_v1( const img::img_type& dst, const img::img_type& src );
    //transform_function_type      get_transform_pwl_to_bayerfloat_neon( img::img_type dst, img::img_type src );

    transform_function_param_type      get_transform_pwl12_to_fcc8_c( const img::img_type& dst, const img::img_type& src );




    void    update_pwl12_to_fcc8_wb_map_data( pwl12_to_fcc8_wb_map_data& data, const img::pwl_transform_params& pwl_params, const img_filter::whitebalance_params& wb_params );


    namespace detail
    {
        void    transform_pwl12_to_fccfloat_wb_c_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params );
        void    transform_pwl12_mipi_to_fccfloat_wb_c_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params );
        void    transform_pwl16H12_to_fccfloat_wb_c_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params );

        void    transform_pwl12_mipi_to_fccfloat_c_v0( img::img_descriptor dst, img::img_descriptor src );
        void    transform_pwl12_to_fccfloat_c_v0( img::img_descriptor dst, img::img_descriptor src );
        void    transform_pwl16H12_to_fccfloat_c_v0( img::img_descriptor dst, img::img_descriptor src );

        void    transform_pwl12_mipi_to_fcc8_c_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params );
        void    transform_pwl12_to_fcc8_c_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params );
        void    transform_pwl16H12_to_fcc8_c_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params );
    }

    transform_function_param_type      get_transform_pwl_to_fccfloat_wb_c( const img::img_type& dst, const img::img_type& src );


    transform_func_type      get_transform_fccfloat_to_fcc8_c( img::img_type dst, img::img_type src );
    transform_func_type      get_transform_fccfloat_to_fcc8_sse41_v0(img::img_type dst, img::img_type src);
    transform_func_type      get_transform_fccfloat_to_fcc8_avx2_v0(img::img_type dst, img::img_type src);
    transform_func_type      get_transform_fccfloat_to_fcc8_neon_v0(img::img_type dst, img::img_type src);

    float       calc_pwl12_pixel_to_float( const void* line_start, int pixel_offset ) noexcept;
    float       calc_pwl12_mipi_pixel_to_float( const void* line_start, int pixel_offset ) noexcept;
    float       calc_pwl16H12_pixel_to_float( const void* line_start, int pixel_offset ) noexcept;
}
}
}
