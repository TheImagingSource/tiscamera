
#pragma once

#include <cstdint>

#include "image_fourcc.h"

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

    constexpr by_pattern	convert_bayer_fcc_to_pattern( uint32_t fcc ) noexcept;
    constexpr uint32_t	    convert_pattern_to_bayer8_fcc( by_pattern pattern ) noexcept;
    constexpr uint32_t	    convert_pattern_to_bayer16_fcc( by_pattern pattern ) noexcept;

    constexpr uint32_t	    convert_bayer_fcc_to_bayer8_fcc( uint32_t byfcc ) noexcept;
    constexpr uint32_t	    convert_bayer_fcc_to_bayer16_fcc( uint32_t byfcc ) noexcept;

    /**
     * Converts Y10/BY10/Y12/BY12 to its equivalent 16 bit variant. (FOURCC_GRBG12_SPACKED => FOURCC_GRBG16)
     * @returns 0 on invalid input fcc
     */
    constexpr uint32_t  convert_packed_fcc10or12_to_fcc16( uint32_t val ) noexcept;

    /** Converts Y10/BY10/Y12/BY12 to its equivalent 8 bit variant. (FOURCC_GRBG12_SPACKED => FOURCC_GRBG8)
     * @returns 0 on invalid input fcc
     */
    constexpr uint32_t  convert_packed_fcc10or12_to_fcc8( uint32_t val ) noexcept;

	/**
	 * Converts PWL compressed format to its equivalent 32 bit float variant. (FOURCC_PWL_RG12_MIPI => FOURCC_RGGBFloat)
	 * @returns 0 on invalid input fcc
	 */
	constexpr uint32_t      convert_pwl_to_fcc32f(uint32_t val) noexcept;

    /**
     * Converts 32 bit float format to corresponding 8 bit variant. (FOURCC_RGGBFloat => FOURCC_RGGB8)
     * @returns 0 on invalid input fcc
     */
    constexpr uint32_t      convert_fcc32f_to_fcc8(uint32_t val) noexcept;

    constexpr by_pattern	convert_bayer_fcc_to_pattern( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
            // is_by8_fcc
        case FOURCC_BGGR8:	return by_pattern::BG;
        case FOURCC_GBRG8:	return by_pattern::GB;
        case FOURCC_RGGB8:	return by_pattern::RG;
        case FOURCC_GRBG8:	return by_pattern::GR;

            // is_by16_fcc
        case FOURCC_BGGR16:	return by_pattern::BG;
        case FOURCC_GBRG16:	return by_pattern::GB;
        case FOURCC_RGGB16:	return by_pattern::RG;
        case FOURCC_GRBG16:	return by_pattern::GR;

            // by16L12
        case FOURCC_BGGR16_L12:	return by_pattern::BG;
        case FOURCC_GBRG16_L12:	return by_pattern::GB;
        case FOURCC_RGGB16_L12:	return by_pattern::RG;
        case FOURCC_GRBG16_L12:	return by_pattern::GR;

            // is_by10_packed_fcc
        case FOURCC_GRBG10_SPACKED:	return by_pattern::GR;
        case FOURCC_RGGB10_SPACKED:	return by_pattern::RG;
        case FOURCC_GBRG10_SPACKED:	return by_pattern::GB;
        case FOURCC_BGGR10_SPACKED:	return by_pattern::BG;
            // is_by10_packed_fcc
        case FOURCC_GRBG10_MIPI_PACKED:	return by_pattern::GR;
        case FOURCC_RGGB10_MIPI_PACKED:	return by_pattern::RG;
        case FOURCC_GBRG10_MIPI_PACKED:	return by_pattern::GB;
        case FOURCC_BGGR10_MIPI_PACKED:	return by_pattern::BG;

            // is_by12_packed_fcc
        case FOURCC_BGGR12_PACKED:	return by_pattern::BG;
        case FOURCC_GBRG12_PACKED:	return by_pattern::GB;
        case FOURCC_RGGB12_PACKED:	return by_pattern::RG;
        case FOURCC_GRBG12_PACKED:	return by_pattern::GR;

        case FOURCC_BGGR12_SPACKED:	return by_pattern::BG;
        case FOURCC_GBRG12_SPACKED:	return by_pattern::GB;
        case FOURCC_RGGB12_SPACKED:	return by_pattern::RG;
        case FOURCC_GRBG12_SPACKED:	return by_pattern::GR;

        case FOURCC_BGGR12_MIPI_PACKED:	return by_pattern::BG;
        case FOURCC_GBRG12_MIPI_PACKED:	return by_pattern::GB;
        case FOURCC_RGGB12_MIPI_PACKED:	return by_pattern::RG;
        case FOURCC_GRBG12_MIPI_PACKED:	return by_pattern::GR;

            // is_byfloat_fcc
        case FOURCC_BGGRFloat:	return by_pattern::BG;
        case FOURCC_GBRGFloat:	return by_pattern::GB;
        case FOURCC_RGGBFloat:	return by_pattern::RG;
        case FOURCC_GRBGFloat:	return by_pattern::GR;

        };

        //assert( false && "input was not a bayer type, so break" );        // input was not a bayer type, so break
        return by_pattern::GB;
    }


    namespace by_pattern_alg {
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

constexpr uint32_t img::by_transform::convert_packed_fcc10or12_to_fcc16( uint32_t val ) noexcept
{
	switch( val )
	{
	case FOURCC_MONO10_PACKED:
	case FOURCC_MONO10_MIPI_PACKED:
	case FOURCC_MONO12_PACKED:
	case FOURCC_MONO12_MIPI_PACKED:
	case FOURCC_MONO12_SPACKED:
	case FOURCC_MONO16_L12:
		return FOURCC_MONO16;

	case FOURCC_GRBG10_MIPI_PACKED:
	case FOURCC_GRBG10_SPACKED:
	case FOURCC_GRBG12_PACKED:
	case FOURCC_GRBG12_SPACKED:
	case FOURCC_GRBG12_MIPI_PACKED:
	case FOURCC_GRBG16_L12:
		return FOURCC_GRBG16;

	case FOURCC_RGGB10_MIPI_PACKED:
	case FOURCC_RGGB10_SPACKED:
	case FOURCC_RGGB12_PACKED:
	case FOURCC_RGGB12_SPACKED:
	case FOURCC_RGGB12_MIPI_PACKED:
	case FOURCC_RGGB16_L12:
		return FOURCC_RGGB16;

	case FOURCC_GBRG10_MIPI_PACKED:
	case FOURCC_GBRG10_SPACKED:
	case FOURCC_GBRG12_PACKED:
	case FOURCC_GBRG12_SPACKED:
	case FOURCC_GBRG12_MIPI_PACKED:
	case FOURCC_GBRG16_L12:
		return FOURCC_GBRG16;

	case FOURCC_BGGR10_MIPI_PACKED:
	case FOURCC_BGGR10_SPACKED:
	case FOURCC_BGGR12_PACKED:
	case FOURCC_BGGR12_SPACKED:
	case FOURCC_BGGR12_MIPI_PACKED:
	case FOURCC_BGGR16_L12:
		return FOURCC_BGGR16;
	default:
		return 0;
	};
}

constexpr uint32_t img::by_transform::convert_packed_fcc10or12_to_fcc8( uint32_t val ) noexcept
{
	switch( val )
	{
	case FOURCC_MONO10_PACKED:          return FOURCC_MONO8;
	case FOURCC_MONO10_MIPI_PACKED:     return FOURCC_MONO8;
	case FOURCC_MONO12_PACKED:          return FOURCC_MONO8;
	case FOURCC_MONO12_MIPI_PACKED:     return FOURCC_MONO8;
	case FOURCC_MONO12_SPACKED:         return FOURCC_MONO8;
    case FOURCC_MONO16_L12:             return FOURCC_MONO8;

	case FOURCC_GRBG10_MIPI_PACKED:     return FOURCC_GRBG8;
	case FOURCC_RGGB10_MIPI_PACKED:     return FOURCC_RGGB8;
	case FOURCC_GBRG10_MIPI_PACKED:     return FOURCC_GBRG8;
	case FOURCC_BGGR10_MIPI_PACKED:     return FOURCC_BGGR8;

	case FOURCC_GRBG10_SPACKED:         return FOURCC_GRBG8;
	case FOURCC_RGGB10_SPACKED:         return FOURCC_RGGB8;
	case FOURCC_GBRG10_SPACKED:         return FOURCC_GBRG8;
	case FOURCC_BGGR10_SPACKED:         return FOURCC_BGGR8;

	case FOURCC_GRBG12_PACKED:          return FOURCC_GRBG8;
	case FOURCC_RGGB12_PACKED:          return FOURCC_RGGB8;
	case FOURCC_GBRG12_PACKED:          return FOURCC_GBRG8;
	case FOURCC_BGGR12_PACKED:          return FOURCC_BGGR8;

	case FOURCC_GRBG12_SPACKED:         return FOURCC_GRBG8;
	case FOURCC_RGGB12_SPACKED:         return FOURCC_RGGB8;
	case FOURCC_GBRG12_SPACKED:         return FOURCC_GBRG8;
	case FOURCC_BGGR12_SPACKED:         return FOURCC_BGGR8;

	case FOURCC_GRBG12_MIPI_PACKED:     return FOURCC_GRBG8;
	case FOURCC_RGGB12_MIPI_PACKED:     return FOURCC_RGGB8;
	case FOURCC_GBRG12_MIPI_PACKED:     return FOURCC_GBRG8;
	case FOURCC_BGGR12_MIPI_PACKED:     return FOURCC_BGGR8;

    case FOURCC_GRBG16_L12:     return FOURCC_GRBG8;
    case FOURCC_RGGB16_L12:     return FOURCC_RGGB8;
    case FOURCC_GBRG16_L12:     return FOURCC_GBRG8;
    case FOURCC_BGGR16_L12:     return FOURCC_BGGR8;
	};
	return 0;
}


constexpr uint32_t img::by_transform::convert_pwl_to_fcc32f( uint32_t val ) noexcept
{
    switch( val )
    {
    case FOURCC_PWL_MONO12_MIPI:
        return FOURCC_MONOFloat;

    case FOURCC_PWL_RG12_MIPI:
        return FOURCC_RGGBFloat;

    default:
        return 0;
    };
}

constexpr uint32_t img::by_transform::convert_fcc32f_to_fcc8(uint32_t val) noexcept
{
    switch (val)
    {
    case FOURCC_MONOFloat:
        return FOURCC_MONO8;
    case FOURCC_BGGRFloat:
        return FOURCC_BGGR8;
    case FOURCC_GBRGFloat:
        return FOURCC_GBRG8;
    case FOURCC_GRBGFloat:
        return FOURCC_GRBG8;
    case FOURCC_RGGBFloat:
        return FOURCC_RGGB8;
    default:
        return 0;
    };
}

constexpr uint32_t	img::by_transform::convert_pattern_to_bayer8_fcc( by_pattern pat ) noexcept
{
    switch( pat )
    {
    case by_pattern::BG: return FOURCC_BGGR8;
    case by_pattern::GB: return FOURCC_GBRG8;
    case by_pattern::RG: return FOURCC_RGGB8;
    case by_pattern::GR: return FOURCC_GRBG8;
    }
    return 0;
}


constexpr uint32_t	img::by_transform::convert_pattern_to_bayer16_fcc( by_pattern pat ) noexcept
{
    switch( pat )
    {
    case by_pattern::BG: return FOURCC_BGGR16;
    case by_pattern::GB: return FOURCC_GBRG16;
    case by_pattern::RG: return FOURCC_RGGB16;
    case by_pattern::GR: return FOURCC_GRBG16;
    }
    return 0;
}

constexpr uint32_t	img::by_transform::convert_bayer_fcc_to_bayer8_fcc( uint32_t byfcc ) noexcept
{
    return convert_pattern_to_bayer8_fcc( convert_bayer_fcc_to_pattern( byfcc ) );
}

constexpr uint32_t	img::by_transform::convert_bayer_fcc_to_bayer16_fcc( uint32_t byfcc ) noexcept
{
    return convert_pattern_to_bayer16_fcc( convert_bayer_fcc_to_pattern( byfcc ) );
}