

#pragma once

#include "image_fourcc_enum.h"

#include <string>

namespace img
{
    inline std::string      fcc_to_string( uint32_t fcc );		// This returns a more descriptive string
    inline std::string      fcc_to_FCCstring( uint32_t fcc );	// converts to either the string BGR* or the actual fourcc chars

	inline std::string      fcc_to_FCCstring( uint32_t fcc )
    {
        switch( fcc )
        {
        case FOURCC_BGR24:  return "BGR24";
        case FOURCC_BGRA32: return "BGRA32";
        case FOURCC_BGRA64: return "BGRA64";
        case 0:             return "NULL";
        default:
            break;
        }

        const char fccBuf[5] = {
            static_cast<char>((fcc >> 0) & 0xff),
            static_cast<char>((fcc >> 8) & 0xff),
            static_cast<char>((fcc >> 16) & 0xff),
            static_cast<char>((fcc >> 24) & 0xff),
            '\0'
        };
        return fccBuf;
    }

	inline std::string      fcc_to_string( uint32_t fcc )
	{
		switch( (fourcc) fcc )
		{
		case fourcc::BGR24:							return "BGR24";
		case fourcc::BGRA32:						return "BGRA32";
		case fourcc::BGRA64:						return "BGRA64";
		case fourcc::BGRFloat:						return "BGRFloat";

		case fourcc::RAW8:							return "Raw8";
		case fourcc::RAW16:							return "Raw16";
		case fourcc::RAW32:							return "Raw32";

		case fourcc::YUY2:							return "YUY2";
		case fourcc::UYVY:							return "UYVY";
		case fourcc::YV16:							return "YV16";
		case fourcc::I420:							return "I420";
		case fourcc::IYU1:							return "IYU1";
		case fourcc::Y411:							return "Y411";
		case fourcc::IYU2:							return "IYU2";
		case fourcc::NV12:							return "NV12";

		case fourcc::MJPG:							return "MJPG";

		case fourcc::HSV24:							return "HSV24";
		case fourcc::HSVx32:						return "HSVx32";

		case fourcc::MONO8:							return "Mono8";
		case fourcc::MONO16:						return "Mono16";
        case fourcc::MONO16_L12:					return "Mono16 12b data";
        case fourcc::MONOFloat:						return "Mono float";

		case fourcc::MONO10_PACKED:                 return "MONO10_PACKED";
        case fourcc::MONO10_MIPI_PACKED:            return "MONO10_MIPI_PACKED";
		case fourcc::MONO12_PACKED:                 return "MONO12_PACKED";
		case fourcc::MONO12_MIPI_PACKED:            return "MONO12_MIPI_PACKED";
		case fourcc::MONO12_SPACKED:                return "MONO12_SPACKED";

		case fourcc::BGGR8:							return "BGGR8";
		case fourcc::GBRG8:							return "GBRG8";
		case fourcc::RGGB8:							return "RGGB8";
		case fourcc::GRBG8:							return "GRBG8";

		case fourcc::BGGR16:		                return "BGGR16";
		case fourcc::GBRG16:		                return "GBRG16";
		case fourcc::GRBG16:		                return "GRBG16";
		case fourcc::RGGB16:		                return "RGGB16";

		case fourcc::BGGRFloat:		                return "BGGRFloat";
		case fourcc::GBRGFloat:		                return "GBRGFloat";
		case fourcc::GRBGFloat:		                return "GRBGFloat";
		case fourcc::RGGBFloat:		                return "RGGBFloat";

        case fourcc::BGGR16_L12:		            return "BGGR16_L12";
        case fourcc::GBRG16_L12:		            return "GBRG16_L12";
        case fourcc::GRBG16_L12:		            return "GRBG16_L12";
        case fourcc::RGGB16_L12:		            return "RGGB16_L12";

        case fourcc::GRBG10_MIPI_PACKED:		    return "GRBG10_MIPI_PACKED";
        case fourcc::RGGB10_MIPI_PACKED:		    return "RGGB10_MIPI_PACKED";
        case fourcc::GBRG10_MIPI_PACKED:		    return "GBRG10_MIPI_PACKED";
        case fourcc::BGGR10_MIPI_PACKED:		    return "BGGR10_MIPI_PACKED";

		case fourcc::GRBG10_SPACKED:		        return "GRBG10_SPACKED";
		case fourcc::RGGB10_SPACKED:		        return "RGGB10_SPACKED";
		case fourcc::GBRG10_SPACKED:		        return "GBRG10_SPACKED";
		case fourcc::BGGR10_SPACKED:		        return "BGGR10_SPACKED";

		case fourcc::GRBG12_PACKED:		        return "GRBG12_PACKED";
		case fourcc::RGGB12_PACKED:		        return "RGGB12_PACKED";
		case fourcc::GBRG12_PACKED:		        return "GBRG12_PACKED";
		case fourcc::BGGR12_PACKED:		        return "BGGR12_PACKED";

		case fourcc::GRBG12_MIPI_PACKED:		    return "GRBG12_MIPI_PACKED";
		case fourcc::RGGB12_MIPI_PACKED:		    return "RGGB12_MIPI_PACKED";
		case fourcc::GBRG12_MIPI_PACKED:		    return "GBRG12_MIPI_PACKED";
		case fourcc::BGGR12_MIPI_PACKED:		    return "BGGR12_MIPI_PACKED";

		case fourcc::GRBG12_SPACKED:             return "GRBG12_SPACKED";
		case fourcc::RGGB12_SPACKED:             return "RGGB12_SPACKED";
		case fourcc::GBRG12_SPACKED:             return "GBRG12_SPACKED";
		case fourcc::BGGR12_SPACKED:             return "BGGR12_SPACKED";

		case fourcc::YUV8PLANAR:	                    return "YUV8 planar";
		case fourcc::YUV16PLANAR:                    return "YUV16 planar";
		case fourcc::YUVF32PLANAR:                   return "YUV32 planar";

			// Polarization formats:
		case fourcc::POLARIZATION_MONO8_90_45_135_0:			return "Polarization Mono8 90 45 135 0";
		case fourcc::POLARIZATION_MONO16_90_45_135_0:			return "Polarization Mono16 90 45 135 0";
		case fourcc::POLARIZATION_MONO12_PACKED_90_45_135_0:	return "Polarization Mono12 packed 90 45 135 0";
		case fourcc::POLARIZATION_MONO12_SPACKED_90_45_135_0:	return "Polarization Mono12 spacked 90 45 135 0";

		case fourcc::POLARIZATION_BG8_90_45_135_0:				return "Polarization Bayer8 90 45 135 0";
		case fourcc::POLARIZATION_BG16_90_45_135_0:				return "Polarization Bayer16 90 45 135 0";
		case fourcc::POLARIZATION_BG12_PACKED_90_45_135_0:		return "Polarization Bayer12 packed 90 45 135 0";
		case fourcc::POLARIZATION_BG12_SPACKED_90_45_135_0:		return "Polarization Bayer12 spacked 90 45 135 0";

		case fourcc::POLARIZATION_ADI_MONO8:					return "Polarization ADI Mono8";
		case fourcc::POLARIZATION_ADI_MONO16:					return "Polarization ADI Mono16";
		case fourcc::POLARIZATION_ADI_PLANAR_MONO8:				return "Polarization ADI Mono8 planar";
		case fourcc::POLARIZATION_ADI_PLANAR_MONO16:			return "Polarization ADI Mono16 planar";
		case fourcc::POLARIZATION_ADI_RGB8:						return "Polarization ADI RGB8";
		case fourcc::POLARIZATION_ADI_RGB16:					return "Polarization ADI RGB16";

		case fourcc::POLARIZATION_PACKED8:						return "Polarization Packed8";
		case fourcc::POLARIZATION_PACKED16:						return "Polarization Packed16";
		case fourcc::POLARIZATION_PACKED8_BAYER_BG:				return "Polarization Packed8 Bayer BG";
		case fourcc::POLARIZATION_PACKED16_BAYER_BG:			return "Polarization Packed16 Bayer BG";

			// PWL formats:
		case fourcc::PWL_MONO12_MIPI:					return "PWL Mono 12-bit mipi-packed";
		case fourcc::PWL_RG12_MIPI:						return "PWL RGGB 12-bit mipi-packed";

		};

		return fcc_to_FCCstring( fcc );
	}
}
