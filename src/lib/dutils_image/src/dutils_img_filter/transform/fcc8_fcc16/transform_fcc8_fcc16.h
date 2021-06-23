
#pragma once

#include "../transform_base.h"

namespace img_filter::transform
{
    constexpr img::fourcc    convert_fcc8_to_fcc16( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR8: return img::fourcc::BGGR16;
        case img::fourcc::GBRG8: return img::fourcc::GBRG16;
        case img::fourcc::RGGB8: return img::fourcc::RGGB16;
        case img::fourcc::GRBG8: return img::fourcc::GRBG16;

        case img::fourcc::MONO8: return img::fourcc::MONO16;
        case img::fourcc::RAW8: return img::fourcc::RAW16;
        default:
            return img::fourcc::FCC_NULL;
        };
    }

    constexpr img::fourcc    convert_fcc16_to_fcc8( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR16: return img::fourcc::BGGR8;
        case img::fourcc::GBRG16: return img::fourcc::GBRG8;
        case img::fourcc::RGGB16: return img::fourcc::RGGB8;
        case img::fourcc::GRBG16: return img::fourcc::GRBG8;

        case img::fourcc::MONO16: return img::fourcc::MONO8;
        case img::fourcc::RAW16: return img::fourcc::RAW8;
        default:
            return img::fourcc::FCC_NULL;
        };
    }

    constexpr bool can_convert_fcc16_to_fcc8( img::img_type dst, img::img_type src )
    {
        return convert_fcc16_to_fcc8( src.fourcc_type() ) == dst.fourcc_type();
    }
    constexpr bool can_convert_fcc8_to_fcc16( img::img_type dst, img::img_type src )
    {
        return convert_fcc8_to_fcc16( src.fourcc_type() ) == dst.fourcc_type();
    }

    transform_function_type      get_transform_fcc8_to_fcc16_c( const img::img_type& dst, const img::img_type& src );
    transform_function_type      get_transform_fcc8_to_fcc16_sse41( const img::img_type& dst, const img::img_type& src );
    transform_function_type      get_transform_fcc8_to_fcc16_neon( const img::img_type& dst, const img::img_type& src );

    transform_function_type      get_transform_fcc16_to_fcc8_c( const img::img_type& dst, const img::img_type& src );
    transform_function_type      get_transform_fcc16_to_fcc8_sse41( const img::img_type& dst, const img::img_type& src );
    transform_function_type      get_transform_fcc16_to_fcc8_neon( const img::img_type& dst, const img::img_type& src );

    transform_function_param_type   get_transform_fcc16_to_fcc8_wb_neon( const img::img_type& dst, const img::img_type& src );
}
