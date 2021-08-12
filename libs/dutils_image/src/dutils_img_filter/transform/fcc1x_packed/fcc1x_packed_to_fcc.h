
#pragma once

#include "../transform_base.h"
#include <dutils_img/image_bayer_pattern.h>


namespace img_filter {
namespace transform {
namespace fcc1x_packed
{
    /**
     * Converts Y10/BY10/Y12/BY12 to its equivalent 16 bit variant. (img::fourcc::GRBG12_SPACKED => img::fourcc::GRBG16)
     * @returns img::fourcc::FCC_NULL on invalid input fcc
     */
    constexpr img::fourcc   convert_packed_fcc1x_to_fcc8( img::fourcc val ) noexcept;
    /**
     * Converts Y10/BY10/Y12/BY12 to its equivalent 8 bit variant. (img::fourcc::GRBG12_SPACKED => img::fourcc::GRBG8)
     * @returns img::fourcc::FCC_NULL on invalid input fcc
     */
    constexpr img::fourcc   convert_packed_fcc1x_to_fcc16( img::fourcc val ) noexcept;

    // Note: this transforms any of the fcc10/fcc12/or fcc16L12 formats to its equiv fcc16 format
    transform_function_type     get_transform_fcc10or12_packed_to_fcc16_c( const img::img_type& dst, const img::img_type& src );
    transform_function_type     get_transform_fcc10or12_packed_to_fcc16_ssse3( const img::img_type& dst, const img::img_type& src );
    transform_function_type     get_transform_fcc10or12_packed_to_fcc16_neon_v0( const img::img_type& dst, const img::img_type& src );

	transform_function_type     get_transform_fcc10or12_packed_to_fcc8_c( const img::img_type& dst, const img::img_type& src );
	transform_function_type     get_transform_fcc10or12_packed_to_fcc8_ssse3( const img::img_type& dst, const img::img_type& src );
	transform_function_type     get_transform_fcc10or12_packed_to_fcc8_neon_v0( const img::img_type& dst, const img::img_type& src );

}
}
}

constexpr img::fourcc   img_filter::transform::fcc1x_packed::convert_packed_fcc1x_to_fcc8( img::fourcc val ) noexcept
{
    switch( val )
    {
    case img::fourcc::MONO10:
    case img::fourcc::MONO10_SPACKED:
    case img::fourcc::MONO10_MIPI_PACKED:
    case img::fourcc::MONO12:
    case img::fourcc::MONO12_PACKED:
    case img::fourcc::MONO12_MIPI_PACKED:
    case img::fourcc::MONO12_SPACKED:
        return img::fourcc::MONO8;

    case img::fourcc::GRBG10:
    case img::fourcc::GRBG10_MIPI_PACKED:
    case img::fourcc::GRBG10_SPACKED:
    case img::fourcc::GRBG12:
    case img::fourcc::GRBG12_PACKED:
    case img::fourcc::GRBG12_SPACKED:
    case img::fourcc::GRBG12_MIPI_PACKED:
        return img::fourcc::GRBG8;

    case img::fourcc::RGGB10:
    case img::fourcc::RGGB10_MIPI_PACKED:
    case img::fourcc::RGGB10_SPACKED:
    case img::fourcc::RGGB12:
    case img::fourcc::RGGB12_PACKED:
    case img::fourcc::RGGB12_SPACKED:
    case img::fourcc::RGGB12_MIPI_PACKED:
        return img::fourcc::RGGB8;

    case img::fourcc::GBRG10:
    case img::fourcc::GBRG10_MIPI_PACKED:
    case img::fourcc::GBRG10_SPACKED:
    case img::fourcc::GBRG12:
    case img::fourcc::GBRG12_PACKED:
    case img::fourcc::GBRG12_SPACKED:
    case img::fourcc::GBRG12_MIPI_PACKED:
        return img::fourcc::GBRG8;

    case img::fourcc::BGGR10:
    case img::fourcc::BGGR10_MIPI_PACKED:
    case img::fourcc::BGGR10_SPACKED:
    case img::fourcc::BGGR12:
    case img::fourcc::BGGR12_PACKED:
    case img::fourcc::BGGR12_SPACKED:
    case img::fourcc::BGGR12_MIPI_PACKED:
        return img::fourcc::BGGR8;

    default:
        return img::fourcc::FCC_NULL;
    };
}


constexpr img::fourcc img_filter::transform::fcc1x_packed::convert_packed_fcc1x_to_fcc16( img::fourcc val ) noexcept
{
    switch( val )
    {
    case img::fourcc::MONO10:
    case img::fourcc::MONO10_SPACKED:
    case img::fourcc::MONO10_MIPI_PACKED:
    case img::fourcc::MONO12:
    case img::fourcc::MONO12_PACKED:
    case img::fourcc::MONO12_MIPI_PACKED:
    case img::fourcc::MONO12_SPACKED:
        return img::fourcc::MONO16;

    case img::fourcc::GRBG10:
    case img::fourcc::GRBG10_MIPI_PACKED:
    case img::fourcc::GRBG10_SPACKED:
    case img::fourcc::GRBG12:
    case img::fourcc::GRBG12_PACKED:
    case img::fourcc::GRBG12_SPACKED:
    case img::fourcc::GRBG12_MIPI_PACKED:
        return img::fourcc::GRBG16;

    case img::fourcc::RGGB10:
    case img::fourcc::RGGB10_MIPI_PACKED:
    case img::fourcc::RGGB10_SPACKED:
    case img::fourcc::RGGB12:
    case img::fourcc::RGGB12_PACKED:
    case img::fourcc::RGGB12_SPACKED:
    case img::fourcc::RGGB12_MIPI_PACKED:
        return img::fourcc::RGGB16;

    case img::fourcc::GBRG10:
    case img::fourcc::GBRG10_MIPI_PACKED:
    case img::fourcc::GBRG10_SPACKED:
    case img::fourcc::GBRG12:
    case img::fourcc::GBRG12_PACKED:
    case img::fourcc::GBRG12_SPACKED:
    case img::fourcc::GBRG12_MIPI_PACKED:
        return img::fourcc::GBRG16;

    case img::fourcc::BGGR10:
    case img::fourcc::BGGR10_MIPI_PACKED:
    case img::fourcc::BGGR10_SPACKED:
    case img::fourcc::BGGR12:
    case img::fourcc::BGGR12_PACKED:
    case img::fourcc::BGGR12_SPACKED:
    case img::fourcc::BGGR12_MIPI_PACKED:
        return img::fourcc::BGGR16;
    default:
        return img::fourcc::FCC_NULL;
    };
}
