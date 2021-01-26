
#pragma once

#include "image_fourcc.h"
#include "image_fourcc_enum.h"
#include "image_helper_types.h"

#include <cstdint>  // uint32_t
#include <utility>  // initializer list
#include <cassert>

namespace img
{
    constexpr int	get_bits_per_pixel( uint32_t fcc ) noexcept;
    constexpr int	get_bits_per_pixel( fourcc fcc ) noexcept;

    constexpr bool  is_known_fcc( uint32_t fcc ) noexcept;
    constexpr bool  is_bottom_up_fcc( uint32_t fcc ) noexcept;
    constexpr bool  is_multi_plane_format( uint32_t fcc ) noexcept;

    constexpr bool  is_fcc_in_fcclist( uint32_t type, std::initializer_list<uint32_t> typelist ) noexcept;

    constexpr bool	is_yuv_format( uint32_t fcc ) noexcept;

    constexpr bool	is_by8_fcc( uint32_t fcc ) noexcept;
    constexpr bool	is_by10_packed_fcc( uint32_t fcc ) noexcept;
    constexpr bool	is_by12_packed_fcc( uint32_t fcc ) noexcept;
    constexpr bool	is_by16_fcc( uint32_t fcc ) noexcept;
    constexpr bool	is_by16L12_fcc( uint32_t fcc ) noexcept;
    constexpr bool	is_byfloat_fcc( uint32_t fcc ) noexcept;

    constexpr bool	is_by10or12or16L12_fcc( uint32_t fcc ) noexcept;

    constexpr bool	is_y10_packed_fcc( uint32_t fcc ) noexcept;
    constexpr bool	is_y12_packed_fcc( uint32_t fcc ) noexcept;
    
    constexpr bool	is_y10or12or16L12_fcc( uint32_t fcc ) noexcept;

    constexpr bool  is_polarization_cam_format( uint32_t fcc ) noexcept;

    constexpr bool  is_bayer_fcc( uint32_t fcc ) noexcept;


    constexpr int   calc_minimum_pitch( uint32_t fcc, int dim_x ) noexcept;
    constexpr int   calc_minimum_img_size( uint32_t fcc, img::dim dim ) noexcept;
    constexpr int   calc_img_size_from_bpp( img::dim dim, int bpp ) noexcept;

    constexpr int   calc_plane_pitch( uint32_t type, int dim_y, int line_pitch ) noexcept;
    constexpr int   calc_planar_fmt_count_of_planes( uint32_t fcc ) noexcept; // == 1 for all packed formats, otherwise the count of planes in this format

    constexpr int	get_bits_per_pixel( uint32_t fcc ) noexcept { return get_bits_per_pixel( static_cast<fourcc>(fcc) ); }
    constexpr int	get_bits_per_pixel( fourcc fcc ) noexcept
	{
		switch( fcc )
		{
		case fourcc::BGR24:		return 24;
		case fourcc::BGRA32:	return 32;
		case fourcc::BGRA64:    return 64;
        
        case fourcc::BGRFloat:      return 96;

		case fourcc::YUY2:		return 16;
		case fourcc::UYVY:		return 16;
		case fourcc::YV16:		return 16;  // planar
		case fourcc::I420:		return 12;  // planar
		case fourcc::IYU1:      return 12;
		case fourcc::Y411:      return 12;      // the same format as IYU1
		case fourcc::IYU2:      return 24;
        case fourcc::NV12:      return 12;

		case fourcc::RAW8:		return 8;
		case fourcc::RAW16:		return 16;
        case fourcc::RAW32:	    return 32;

		case fourcc::MONO8:		        return 8;
		case fourcc::MONO16:	        return 16;
		case fourcc::MONO16_L12:        return 16;
        case fourcc::MONOFloat:	        return 32;

		case fourcc::MONO10_PACKED:                 return 10;
		case fourcc::MONO10_MIPI_PACKED:            return 10;

		case fourcc::MONO12_PACKED:                 return 12;
		case fourcc::MONO12_MIPI_PACKED:            return 12;
		case fourcc::MONO12_SPACKED:                return 12;

		case fourcc::BGGR8:		                return 8;
		case fourcc::GBRG8:		                return 8;
		case fourcc::RGGB8:		                return 8;
		case fourcc::GRBG8:		                return 8;

		case fourcc::BGGR16:		                return 16;
		case fourcc::GBRG16:		                return 16;
		case fourcc::GRBG16:		                return 16;
		case fourcc::RGGB16:		                return 16;

		case fourcc::BGGRFloat:		                return 32;
		case fourcc::GBRGFloat:		                return 32;
		case fourcc::GRBGFloat:		                return 32;
		case fourcc::RGGBFloat:		                return 32;

		case fourcc::BGGR16_L12:		            return 16;
		case fourcc::GBRG16_L12:		            return 16;
		case fourcc::GRBG16_L12:		            return 16;
		case fourcc::RGGB16_L12:		            return 16;

		case fourcc::GRBG10_MIPI_PACKED:		    return 10;
		case fourcc::RGGB10_MIPI_PACKED:		    return 10;
		case fourcc::GBRG10_MIPI_PACKED:		    return 10;
		case fourcc::BGGR10_MIPI_PACKED:		    return 10;

		case fourcc::GRBG10_SPACKED:		        return 10;
		case fourcc::RGGB10_SPACKED:		        return 10;
		case fourcc::GBRG10_SPACKED:		        return 10;
		case fourcc::BGGR10_SPACKED:		        return 10;

		case fourcc::GRBG12_PACKED:		        return 12;
		case fourcc::RGGB12_PACKED:		        return 12;
		case fourcc::GBRG12_PACKED:		        return 12;
		case fourcc::BGGR12_PACKED:		        return 12;

		case fourcc::GRBG12_MIPI_PACKED:		    return 12;
		case fourcc::RGGB12_MIPI_PACKED:		    return 12;
		case fourcc::GBRG12_MIPI_PACKED:		    return 12;
		case fourcc::BGGR12_MIPI_PACKED:		    return 12;

		case fourcc::GRBG12_SPACKED:             return 12;
		case fourcc::RGGB12_SPACKED:             return 12;
		case fourcc::GBRG12_SPACKED:             return 12;
		case fourcc::BGGR12_SPACKED:             return 12;

		case fourcc::YUV8PLANAR:	                    return 24;
		case fourcc::YUV16PLANAR:                    return 48;
		case fourcc::YUVF32PLANAR:                   return 96;

		case fourcc::MJPG:                           return 24;

		case fourcc::POLARIZATION_MONO8_90_45_135_0:             return 8;
		case fourcc::POLARIZATION_MONO16_90_45_135_0:            return 16;
		case fourcc::POLARIZATION_MONO12_PACKED_90_45_135_0:     return 12;
		case fourcc::POLARIZATION_MONO12_SPACKED_90_45_135_0:    return 12;

		case fourcc::POLARIZATION_BG8_90_45_135_0:           return 8;
		case fourcc::POLARIZATION_BG16_90_45_135_0:          return 16;
		case fourcc::POLARIZATION_BG12_PACKED_90_45_135_0:   return 12;
		case fourcc::POLARIZATION_BG12_SPACKED_90_45_135_0:  return 12;

		case fourcc::POLARIZATION_ADI_MONO8:                 return 32;
		case fourcc::POLARIZATION_ADI_MONO16:                return 64;
		case fourcc::POLARIZATION_ADI_PLANAR_MONO8:          return 32;
		case fourcc::POLARIZATION_ADI_PLANAR_MONO16:         return 64;
		case fourcc::POLARIZATION_ADI_RGB8:                  return 64;
		case fourcc::POLARIZATION_ADI_RGB16:                 return 128;

		case fourcc::POLARIZATION_PACKED8:            return 32;
		case fourcc::POLARIZATION_PACKED16:           return 64;
		case fourcc::POLARIZATION_PACKED8_BAYER_BG:   return 32;
		case fourcc::POLARIZATION_PACKED16_BAYER_BG:  return 64;

		case fourcc::HSV24:                          return 24;
		case fourcc::HSVx32:                         return 32;

		case fourcc::PWL_MONO12_MIPI:                 return 12;
		case fourcc::PWL_RG12_MIPI:                   return 12;
		}

		return 0;
	}

    constexpr bool		is_yuv_format( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_YUY2:		return true;
        case FOURCC_UYVY:		return true;

        case FOURCC_YV16:		return true;
        case FOURCC_I420:		return true;
        case FOURCC_YUV8PLANAR:	return true;

        case FOURCC_IYU1:       return true;
        case FOURCC_IYU2:       return true;

        case FOURCC_YUV16PLANAR:    return true;
        case FOURCC_YUVF32PLANAR:   return true;
        default:
            return false;
        }
    }

    constexpr bool		is_by16_fcc( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_BGGR16:
        case FOURCC_GBRG16:
        case FOURCC_RGGB16:
        case FOURCC_GRBG16:
            return true;
        default:
            return false;
        };
    }
    constexpr bool		is_by16L12_fcc( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_BGGR16_L12:
        case FOURCC_GBRG16_L12:
        case FOURCC_RGGB16_L12:
        case FOURCC_GRBG16_L12:
            return true;
        default:
            return false;
        };
    }
    constexpr bool	is_byfloat_fcc( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_BGGRFloat:
        case FOURCC_GBRGFloat:
        case FOURCC_RGGBFloat:
        case FOURCC_GRBGFloat:
            return true;
        default:
            return false;
        };
    }

    constexpr bool		is_by8_fcc( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_BGGR8:
        case FOURCC_GBRG8:
        case FOURCC_RGGB8:
        case FOURCC_GRBG8:
            return true;
        default:
            return false;
        };
    }
    constexpr bool		is_by12_packed_fcc( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_BGGR12_PACKED:
        case FOURCC_GBRG12_PACKED:
        case FOURCC_RGGB12_PACKED:
        case FOURCC_GRBG12_PACKED:

        case FOURCC_BGGR12_MIPI_PACKED:
        case FOURCC_GBRG12_MIPI_PACKED:
        case FOURCC_RGGB12_MIPI_PACKED:
        case FOURCC_GRBG12_MIPI_PACKED:

        case FOURCC_BGGR12_SPACKED:
        case FOURCC_GBRG12_SPACKED:
        case FOURCC_RGGB12_SPACKED:
        case FOURCC_GRBG12_SPACKED:
            return true;
        default:
            return false;
        };
    }

    constexpr bool		is_by10_packed_fcc( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_BGGR10_SPACKED:
        case FOURCC_GBRG10_SPACKED:
        case FOURCC_RGGB10_SPACKED:
        case FOURCC_GRBG10_SPACKED:
            return true;
        case FOURCC_BGGR10_MIPI_PACKED:
        case FOURCC_GBRG10_MIPI_PACKED:
        case FOURCC_RGGB10_MIPI_PACKED:
        case FOURCC_GRBG10_MIPI_PACKED:
            return true;
        default:
            return false;
        };
    }
    constexpr bool		is_by10or12or16L12_fcc( uint32_t fcc ) noexcept { 
        return img::is_by10_packed_fcc( fcc ) || img::is_by12_packed_fcc( fcc ) || img::is_by16L12_fcc( fcc );
    }

    constexpr bool		is_y10_packed_fcc( uint32_t fcc ) noexcept { return fcc == FOURCC_MONO10_PACKED || fcc == FOURCC_MONO10_MIPI_PACKED; }
    constexpr bool		is_y12_packed_fcc( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_MONO12_PACKED:
        case FOURCC_MONO12_MIPI_PACKED:
        case FOURCC_MONO12_SPACKED:
            return true;
        default:
            return false;
        };
    }
    constexpr bool		is_y10or12or16L12_fcc( uint32_t fcc ) noexcept { return img::is_y10_packed_fcc( fcc ) || img::is_y12_packed_fcc( fcc ) || fcc == FOURCC_MONO16_L12; }

    constexpr bool      is_polarization_cam_format( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_POLARIZATION_MONO8_90_45_135_0:             return true;
        case FOURCC_POLARIZATION_MONO16_90_45_135_0:            return true;
        case FOURCC_POLARIZATION_MONO12_PACKED_90_45_135_0:     return true;
        case FOURCC_POLARIZATION_MONO12_SPACKED_90_45_135_0:    return true;

        case FOURCC_POLARIZATION_BG8_90_45_135_0:               return true;
        case FOURCC_POLARIZATION_BG16_90_45_135_0:              return true;
        case FOURCC_POLARIZATION_BG12_PACKED_90_45_135_0:       return true;
        case FOURCC_POLARIZATION_BG12_SPACKED_90_45_135_0:      return true;
        }
        return false;
    }
    constexpr bool      is_pwl_fcc( uint32_t fcc ) noexcept
    {
        return fcc == FOURCC_PWL_MONO12_MIPI || fcc == FOURCC_PWL_RG12_MIPI;
    }

    constexpr bool      is_bayer_fcc( uint32_t fcc ) noexcept {
        return is_by8_fcc( fcc ) || is_by16_fcc( fcc ) || is_by10_packed_fcc( fcc ) || is_by12_packed_fcc( fcc ) || is_by16L12_fcc( fcc ) || is_byfloat_fcc( fcc );
    }

    constexpr bool      is_bottom_up_fcc( uint32_t fcc ) noexcept { return fcc == FOURCC_BGR24 || fcc == FOURCC_BGRA32 || fcc == FOURCC_BGRA64; }
    constexpr bool      is_known_fcc( uint32_t fcc ) noexcept    { return get_bits_per_pixel( fcc ) != 0; }
    constexpr bool      is_multi_plane_format( uint32_t fcc ) noexcept { return calc_planar_fmt_count_of_planes( fcc ) > 1; }


    constexpr int       calc_planar_fmt_count_of_planes( uint32_t fcc ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_NV12:		                    return 2;
        case FOURCC_YV16:		                    return 3;
        case FOURCC_I420:		                    return 3;
        case FOURCC_YUV8PLANAR:	                    return 3;
        case FOURCC_YUV16PLANAR:                    return 3;
        case FOURCC_YUVF32PLANAR:                   return 3;
        case FOURCC_POLARIZATION_ADI_PLANAR_MONO8:  return 4;
        case FOURCC_POLARIZATION_ADI_PLANAR_MONO16: return 4;
        default:
            return 1;
        }
    }

    constexpr int       calc_minimum_pitch( uint32_t fcc, int dim_x ) noexcept
    {
        switch( fcc )
        {
        case FOURCC_YV16:		    return dim_x * 1;       // these are sub-sampled in u and v, so specify the y plane pitch
        case FOURCC_I420:		    return dim_x * 1;       // these are sub-sampled in u and v, so specify the y plane pitch

        case FOURCC_YUV8PLANAR:	    return dim_x * 1;
        case FOURCC_YUV16PLANAR:    return dim_x * 2;
        case FOURCC_YUVF32PLANAR:   return dim_x * 4;

        case FOURCC_POLARIZATION_ADI_PLANAR_MONO8:  return dim_x * 1;
        case FOURCC_POLARIZATION_ADI_PLANAR_MONO16: return dim_x * 2;
        case FOURCC_NV12:           return dim_x;
        }
        const int bpp = get_bits_per_pixel( fcc );
        assert( bpp != 0 );
        return (dim_x * bpp) / 8;
    }
    constexpr int       calc_minimum_img_size( uint32_t fcc, img::dim dim ) noexcept
    {
        assert( img::get_bits_per_pixel( fcc ) != 0 );
        switch( fcc )
        {
        case FOURCC_YV16:	return (dim.cx * dim.cy) + (dim.cx * dim.cy / 2) * 2;     // u and v plane are sub-sampled 2x1, 2 planes
        case FOURCC_I420:	return (dim.cx * dim.cy) + (dim.cx * dim.cy / 4) * 2;     // u and v plane are sub-sampled 2x2, 2 planes
        case FOURCC_NV12:	return (dim.cx * dim.cy) + (dim.cx * dim.cy / 4) * 2;     // u and v plane are sub-sampled 2x2, interleaved in 1 planes
        }
        const int bytes_per_line = calc_minimum_pitch( fcc, dim.cx );
        return bytes_per_line * dim.cy * calc_planar_fmt_count_of_planes( fcc );
    }

    constexpr int       calc_img_size_from_bpp( img::dim dim, int bpp ) noexcept
    {
        const int bytes_per_line = (dim.cx * bpp) / 8;
        return bytes_per_line * dim.cy;
    }

    /**
     * For planar formats, this calculates the plane pitch, for others this returns 0.
     */
    constexpr int      calc_plane_pitch( uint32_t type, int dim_y, int line_pitch ) noexcept
    {
        if( !is_multi_plane_format( type ) ) {
            return 0;
        }

        const int pitch = line_pitch < 0 ? -line_pitch : line_pitch;
        return dim_y * pitch;
    }

    constexpr bool is_fcc_in_fcclist( uint32_t type, std::initializer_list<uint32_t> typelist ) noexcept
    {
        for( auto&& e : typelist ) {
            if( type == e ) {
                return true;
            }
        }
        return false;
    }
}
