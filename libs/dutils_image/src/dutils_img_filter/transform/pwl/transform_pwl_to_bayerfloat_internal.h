

#pragma once

#include "transform_pwl_functions.h"

#include <dutils_img/image_bayer_pattern.h>
#include <cmath>

namespace transform_pwl_internal
{
    uint32_t    transform_pwl_to_int_single_value(int value);
    float       transform_pwl_to_float_single_value(int value);

    const float*    get_lut_for_transform_pwl_to_float();

    constexpr bool can_transform_pwl_to_fcc32f( img::img_type dst, img::img_type src ) noexcept
    {
        return img::by_transform::convert_pwl_to_fcc32f( src.fourcc_type() ) == dst.fourcc_type();
    }
    constexpr bool can_transform_fcc32f_to_fcc8( img::img_type dst, img::img_type src ) noexcept
    {
        return img::by_transform::convert_fcc32f_to_fcc8( src.fourcc_type() ) == dst.fourcc_type();
    }

    constexpr bool can_transform_pwl_to_fcc8( img::img_type dst, img::img_type src ) noexcept
    {
        if( src.fourcc_type() == img::fourcc::PWL_RG12_MIPI ||
            src.fourcc_type() == img::fourcc::PWL_RG12 ||
            src.fourcc_type() == img::fourcc::PWL_RG16H12 ) {
            return dst.fourcc_type() == img::fourcc::RGGB8;
        }
        return false;
    }

    struct internal_transform_params
    {
        float gradient;
    };

    FORCEINLINE constexpr uint8_t transform_RawFloat_to_Raw8_c(float value, const internal_transform_params& internal_params) noexcept
    {
        int rval = (int)(value * internal_params.gradient * 255.f + 0.5f);
        return (uint8_t)CLIP( rval, 0, 255 );
    }

    FORCEINLINE internal_transform_params compute_fccfloat_to_fcc8_parameter(const img::pwl_transform_params& params) noexcept
    {
        const auto gradient = std::pow(10.0f, params.hdr_gain / 20.0f);

        return { gradient };
    }
}
