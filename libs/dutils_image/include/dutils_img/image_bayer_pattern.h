
#pragma once

#include <cstdint>

#include "image_fourcc.h"
#include "image_fourcc_enum.h"

namespace img {
namespace by_transform
{
    enum class by_pattern : int
    {
        BG = 0,
        GB = 1,
        GR = 2,
        RG = 3,
    };

    constexpr by_pattern	convert_bayer_fcc_to_pattern( img::fourcc fcc ) noexcept;

    // #TODO maybe remove the fcc_to_bayerX_fcc functions and replace them with the internal mapping stuff

    constexpr img::fourcc	convert_pattern_to_bayer8_fcc( by_pattern pattern ) noexcept;
    constexpr img::fourcc	convert_pattern_to_bayer16_fcc( by_pattern pattern ) noexcept;

    constexpr img::fourcc	convert_bayer_fcc_to_bayer8_fcc( img::fourcc byfcc ) noexcept;
    constexpr img::fourcc	convert_bayer_fcc_to_bayer16_fcc( img::fourcc byfcc ) noexcept;

	/**
	 * Converts PWL compressed format to its equivalent 32 bit float variant. (img::fourcc::PWL_RG12_MIPI => img::fourcc::RGGBFloat)
	 * @returns 0 on invalid input fcc
	 */
	constexpr img::fourcc      convert_pwl_to_fcc32f( img::fourcc val ) noexcept;

    /**
     * Converts 32 bit float format to corresponding 8 bit variant. (img::fourcc::RGGBFloat => img::fourcc::RGGB8)
     * @returns 0 on invalid input fcc
     */
    constexpr img::fourcc       convert_fcc32f_to_fcc8( img::fourcc val ) noexcept;

    namespace by_pattern_alg
    {
        inline constexpr by_pattern	next_pixel( by_pattern pattern ) noexcept
        {
            switch( pattern )
            {
            case by_pattern::BG:	return by_pattern::GB;
            case by_pattern::GB:	return by_pattern::BG;
            case by_pattern::GR:	return by_pattern::RG;
            case by_pattern::RG:	return by_pattern::GR;
            };
            return by_pattern::BG;
        }
        inline constexpr by_pattern	next_line( by_pattern pattern ) noexcept
        {
            switch( pattern )
            {
            case by_pattern::BG:	return by_pattern::GR;
            case by_pattern::GB:	return by_pattern::RG;
            case by_pattern::GR:	return by_pattern::BG;
            case by_pattern::RG:	return by_pattern::GB;
            };
            return by_pattern::BG;
        }
    }
} // by_transform
} // img


constexpr img::fourcc img::by_transform::convert_pwl_to_fcc32f( img::fourcc val ) noexcept
{
    switch( val )
    {
    case img::fourcc::PWL_RG12_MIPI:        return img::fourcc::RGGBFloat;
    case img::fourcc::PWL_RG12:             return img::fourcc::RGGBFloat;
    case img::fourcc::PWL_RG16H12:          return img::fourcc::RGGBFloat;
    default:
        return img::fourcc::FCC_NULL;
    };
}

constexpr img::fourcc img::by_transform::convert_fcc32f_to_fcc8( img::fourcc val ) noexcept
{
    switch( val )
    {
    case img::fourcc::RAWFloat:       return img::fourcc::RAW8;
    case img::fourcc::MONOFloat:      return img::fourcc::MONO8;
    case img::fourcc::BGGRFloat:      return img::fourcc::BGGR8;
    case img::fourcc::GBRGFloat:      return img::fourcc::GBRG8;
    case img::fourcc::GRBGFloat:      return img::fourcc::GRBG8;
    case img::fourcc::RGGBFloat:      return img::fourcc::RGGB8;
    default:
        return img::fourcc::FCC_NULL;
    };
}

constexpr img::fourcc	img::by_transform::convert_pattern_to_bayer8_fcc( by_pattern pat ) noexcept
{
    switch( pat )
    {
    case by_pattern::BG: return img::fourcc::BGGR8;
    case by_pattern::GB: return img::fourcc::GBRG8;
    case by_pattern::RG: return img::fourcc::RGGB8;
    case by_pattern::GR: return img::fourcc::GRBG8;
    default:
        return img::fourcc::FCC_NULL;
    }
}


constexpr img::fourcc	img::by_transform::convert_pattern_to_bayer16_fcc( by_pattern pat ) noexcept
{
    switch( pat )
    {
    case by_pattern::BG: return img::fourcc::BGGR16;
    case by_pattern::GB: return img::fourcc::GBRG16;
    case by_pattern::RG: return img::fourcc::RGGB16;
    case by_pattern::GR: return img::fourcc::GRBG16;
    default:
        return img::fourcc::FCC_NULL;
    }
}

constexpr img::fourcc	img::by_transform::convert_bayer_fcc_to_bayer8_fcc( img::fourcc byfcc ) noexcept
{
    return convert_pattern_to_bayer8_fcc( convert_bayer_fcc_to_pattern( byfcc ) );
}

constexpr img::fourcc	img::by_transform::convert_bayer_fcc_to_bayer16_fcc( img::fourcc byfcc ) noexcept
{
    return convert_pattern_to_bayer16_fcc( convert_bayer_fcc_to_pattern( byfcc ) );
}

constexpr img::by_transform::by_pattern	    img::by_transform::convert_bayer_fcc_to_pattern( img::fourcc fcc ) noexcept
{
    switch( fcc )
    {
        // is_by8_fcc
    case img::fourcc::BGGR8:	return by_pattern::BG;
    case img::fourcc::GBRG8:	return by_pattern::GB;
    case img::fourcc::RGGB8:	return by_pattern::RG;
    case img::fourcc::GRBG8:	return by_pattern::GR;

        // is_by16_fcc
    case img::fourcc::BGGR16:	return by_pattern::BG;
    case img::fourcc::GBRG16:	return by_pattern::GB;
    case img::fourcc::RGGB16:	return by_pattern::RG;
    case img::fourcc::GRBG16:	return by_pattern::GR;

        // by10
    case img::fourcc::BGGR10:	return by_pattern::BG;
    case img::fourcc::GBRG10:	return by_pattern::GB;
    case img::fourcc::RGGB10:	return by_pattern::RG;
    case img::fourcc::GRBG10:	return by_pattern::GR;

        // by12
    case img::fourcc::BGGR12:	            return by_pattern::BG;
    case img::fourcc::GBRG12:	            return by_pattern::GB;
    case img::fourcc::RGGB12:	            return by_pattern::RG;
    case img::fourcc::GRBG12:	            return by_pattern::GR;

        // is_by10_packed_fcc
    case img::fourcc::GRBG10_SPACKED:	    return by_pattern::GR;
    case img::fourcc::RGGB10_SPACKED:	    return by_pattern::RG;
    case img::fourcc::GBRG10_SPACKED:	    return by_pattern::GB;
    case img::fourcc::BGGR10_SPACKED:	    return by_pattern::BG;
        // is_by10_packed_fcc
    case img::fourcc::GRBG10_MIPI_PACKED:	return by_pattern::GR;
    case img::fourcc::RGGB10_MIPI_PACKED:	return by_pattern::RG;
    case img::fourcc::GBRG10_MIPI_PACKED:	return by_pattern::GB;
    case img::fourcc::BGGR10_MIPI_PACKED:	return by_pattern::BG;

        // is_by12_packed_fcc
    case img::fourcc::BGGR12_PACKED:	    return by_pattern::BG;
    case img::fourcc::GBRG12_PACKED:	    return by_pattern::GB;
    case img::fourcc::RGGB12_PACKED:	    return by_pattern::RG;
    case img::fourcc::GRBG12_PACKED:	    return by_pattern::GR;

    case img::fourcc::BGGR12_SPACKED:	    return by_pattern::BG;
    case img::fourcc::GBRG12_SPACKED:	    return by_pattern::GB;
    case img::fourcc::RGGB12_SPACKED:	    return by_pattern::RG;
    case img::fourcc::GRBG12_SPACKED:	    return by_pattern::GR;

    case img::fourcc::BGGR12_MIPI_PACKED:	return by_pattern::BG;
    case img::fourcc::GBRG12_MIPI_PACKED:	return by_pattern::GB;
    case img::fourcc::RGGB12_MIPI_PACKED:	return by_pattern::RG;
    case img::fourcc::GRBG12_MIPI_PACKED:	return by_pattern::GR;

        // is_byfloat_fcc
    case img::fourcc::BGGRFloat:	        return by_pattern::BG;
    case img::fourcc::GBRGFloat:	        return by_pattern::GB;
    case img::fourcc::RGGBFloat:	        return by_pattern::RG;
    case img::fourcc::GRBGFloat:	        return by_pattern::GR;

    case img::fourcc::PWL_RG12_MIPI:      return by_pattern::RG;
    case img::fourcc::PWL_RG12:           return by_pattern::RG;
    case img::fourcc::PWL_RG16H12:        return by_pattern::RG;
    default:
        return by_pattern::GB;
    };
}
