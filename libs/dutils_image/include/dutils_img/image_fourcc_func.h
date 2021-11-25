
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

    constexpr bool  is_known_fcc( img::fourcc fcc ) noexcept;
    constexpr bool  is_known_fcc( uint32_t ) noexcept;
    constexpr bool  is_bottom_up_fcc( img::fourcc fcc ) noexcept;
    constexpr bool  is_multi_plane_format( img::fourcc fcc ) noexcept;

    constexpr bool  is_fcc_in_fcclist( uint32_t type, std::initializer_list<uint32_t> typelist ) noexcept;
    constexpr bool  is_fcc_in_fcclist( img::fourcc type, std::initializer_list<img::fourcc> typelist ) noexcept;

    constexpr bool	is_yuv_format( img::fourcc fcc ) noexcept;

    constexpr bool	is_by8_fcc( img::fourcc fcc ) noexcept;
    constexpr bool	is_by10_packed_fcc( img::fourcc fcc ) noexcept;
    constexpr bool	is_by12_packed_fcc( img::fourcc fcc ) noexcept;
    constexpr bool	is_by16_fcc( img::fourcc fcc ) noexcept;
    constexpr bool	is_by12_fcc( img::fourcc fcc ) noexcept;
    constexpr bool	is_by10_fcc( img::fourcc fcc ) noexcept;
    constexpr bool	is_byfloat_fcc( img::fourcc fcc ) noexcept;

    constexpr bool  is_mono_fcc( img::fourcc fcc ) noexcept;

    constexpr bool  is_polarization_cam_format( img::fourcc fcc ) noexcept;

    constexpr bool  is_bayer_fcc( img::fourcc fcc ) noexcept;


    constexpr int   calc_minimum_pitch( img::fourcc fcc, int dim_x ) noexcept;
    constexpr int   calc_minimum_img_size( img::fourcc fcc, img::dim dim ) noexcept;
    constexpr int   calc_img_size_from_bpp( img::dim dim, int bpp ) noexcept;

    constexpr int   calc_planar_fmt_count_of_planes( img::fourcc fcc ) noexcept; // == 1 for all packed formats, otherwise the count of planes in this format

    constexpr int	get_bits_per_pixel( uint32_t fcc ) noexcept { return get_bits_per_pixel( static_cast<fourcc>(fcc) ); }
    constexpr int	get_bits_per_pixel( fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case fourcc::FCC_NULL:  return 0;
        case fourcc::BGR24:		return 24;
        case fourcc::BGRA32:	return 32;
        case fourcc::BGRA64:    return 64;
        
        case fourcc::BGRFloat:      return 96;

        case fourcc::YUY2:		return 16;
        case fourcc::UYVY:		return 16;
        case fourcc::IYU1:      return 12;
        case fourcc::Y411:      return 12;      // the same format as IYU1
        case fourcc::IYU2:      return 24;
        case fourcc::NV12:      return 12;
        case fourcc::YV12:      return 12;
        case fourcc::I420:      return 12;

        case fourcc::RAW8:		return 8;
        case fourcc::RAW16:		return 16;
        case fourcc::RAW24:     return 24;
        case fourcc::RAW32:	    return 32;
        case fourcc::RAWFloat:	return 32;

        case fourcc::MONO8:		        return 8;
        case fourcc::MONO16:	        return 16;
        case fourcc::MONO12:            return 16;
        case fourcc::MONO10:            return 16;
        case fourcc::MONOFloat:	        return 32;

        case fourcc::MONO10_SPACKED:        return 10;
        case fourcc::MONO10_MIPI_PACKED:    return 10;

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

        case fourcc::BGGR12:		                return 16;
        case fourcc::GBRG12:		                return 16;
        case fourcc::GRBG12:		                return 16;
        case fourcc::RGGB12:		                return 16;

        case fourcc::BGGR10:		                return 16;
        case fourcc::GBRG10:		                return 16;
        case fourcc::GRBG10:		                return 16;
        case fourcc::RGGB10:	    	            return 16;

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

        case fourcc::PWL_RG12_MIPI:                   return 12;
        case fourcc::PWL_RG12:                          return 16;
        case fourcc::PWL_RG16H12:                       return 16;
        }

        return 0;
    }

#ifdef _MSC_VER
#else
#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wswitch-enum"
//#pragma GCC diagnostic ignored "-Wswitch"
#endif

    constexpr bool		is_yuv_format( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case fourcc::YUY2:		    return true;
        case fourcc::UYVY:		    return true;
        case fourcc::YUV8PLANAR:	return true;
        case fourcc::IYU1:          return true;
        case fourcc::Y411:          return true;
        case fourcc::IYU2:          return true;
        case fourcc::YUV16PLANAR:   return true;
        case fourcc::YUVF32PLANAR:  return true;
        case fourcc::YV12:          return true;
        case fourcc::NV12:          return true;
        case fourcc::I420:          return true;
        default:
            return false;
        }
    }

    constexpr bool		is_by16_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR16:
        case img::fourcc::GBRG16:
        case img::fourcc::RGGB16:
        case img::fourcc::GRBG16:
            return true;
        default:
            return false;
        };
    }
    constexpr bool		is_by10_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR10:
        case img::fourcc::GBRG10:
        case img::fourcc::RGGB10:
        case img::fourcc::GRBG10:
            return true;
        default:
            return false;
        };
    }
    constexpr bool		is_by12_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR12:
        case img::fourcc::GBRG12:
        case img::fourcc::RGGB12:
        case img::fourcc::GRBG12:
            return true;
        default:
            return false;
        };
    }
    constexpr bool	is_byfloat_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGRFloat:
        case img::fourcc::GBRGFloat:
        case img::fourcc::RGGBFloat:
        case img::fourcc::GRBGFloat:
            return true;
        default:
            return false;
        };
    }

    constexpr bool		is_by8_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR8:
        case img::fourcc::GBRG8:
        case img::fourcc::RGGB8:
        case img::fourcc::GRBG8:
            return true;
        default:
            return false;
        };
    }
    constexpr bool		is_by12_packed_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR12_PACKED:
        case img::fourcc::GBRG12_PACKED:
        case img::fourcc::RGGB12_PACKED:
        case img::fourcc::GRBG12_PACKED:
        case img::fourcc::BGGR12_MIPI_PACKED:
        case img::fourcc::GBRG12_MIPI_PACKED:
        case img::fourcc::RGGB12_MIPI_PACKED:
        case img::fourcc::GRBG12_MIPI_PACKED:
        case img::fourcc::BGGR12_SPACKED:
        case img::fourcc::GBRG12_SPACKED:
        case img::fourcc::RGGB12_SPACKED:
        case img::fourcc::GRBG12_SPACKED:
            return true;
        default:
            return false;
        };
    }

    constexpr bool		is_by10_packed_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::BGGR10_SPACKED:
        case img::fourcc::GBRG10_SPACKED:
        case img::fourcc::RGGB10_SPACKED:
        case img::fourcc::GRBG10_SPACKED:
            return true;
        case img::fourcc::BGGR10_MIPI_PACKED:
        case img::fourcc::GBRG10_MIPI_PACKED:
        case img::fourcc::RGGB10_MIPI_PACKED:
        case img::fourcc::GRBG10_MIPI_PACKED:
            return true;
        default:
            return false;
        };
    }

    constexpr bool		is_mono10_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::MONO10:
        case img::fourcc::MONO10_SPACKED:
        case img::fourcc::MONO10_MIPI_PACKED:
            return true;
        default:
            return false;
        }
    }
    constexpr bool		is_mono12_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::MONO12:
        case img::fourcc::MONO12_PACKED:
        case img::fourcc::MONO12_MIPI_PACKED:
        case img::fourcc::MONO12_SPACKED:
            return true;
        default:
            return false;
        }
    }

    constexpr bool		is_mono_fcc( img::fourcc fcc ) noexcept 
    {
        switch( fcc )
        {
        case img::fourcc::MONO8:
        case img::fourcc::MONO16:
        case img::fourcc::MONOFloat:
        case img::fourcc::MONO10:
        case img::fourcc::MONO10_SPACKED:
        case img::fourcc::MONO10_MIPI_PACKED:
        case img::fourcc::MONO12:
        case img::fourcc::MONO12_PACKED:
        case img::fourcc::MONO12_MIPI_PACKED:
        case img::fourcc::MONO12_SPACKED:
            return true;
        default:
            return false;
        }
    }

    constexpr bool      is_polarization_cam_format( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::POLARIZATION_MONO8_90_45_135_0:             return true;
        case img::fourcc::POLARIZATION_MONO16_90_45_135_0:            return true;
        case img::fourcc::POLARIZATION_MONO12_PACKED_90_45_135_0:     return true;
        case img::fourcc::POLARIZATION_MONO12_SPACKED_90_45_135_0:    return true;

        case img::fourcc::POLARIZATION_BG8_90_45_135_0:               return true;
        case img::fourcc::POLARIZATION_BG16_90_45_135_0:              return true;
        case img::fourcc::POLARIZATION_BG12_PACKED_90_45_135_0:       return true;
        case img::fourcc::POLARIZATION_BG12_SPACKED_90_45_135_0:      return true;
        default:
            return false;
        }
    }
    constexpr bool      is_adi_fcc( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::POLARIZATION_ADI_MONO8:
        case img::fourcc::POLARIZATION_ADI_MONO16:
        case img::fourcc::POLARIZATION_ADI_PLANAR_MONO8:
        case img::fourcc::POLARIZATION_ADI_PLANAR_MONO16:
        case img::fourcc::POLARIZATION_ADI_RGB8:
        case img::fourcc::POLARIZATION_ADI_RGB16:
            return true;
        default:
            return false;
        }
    }

    constexpr bool      is_pwl_fcc( img::fourcc fcc ) noexcept
    {
        return fcc == img::fourcc::PWL_RG12_MIPI || fcc == img::fourcc::PWL_RG12 || fcc == img::fourcc::PWL_RG16H12;
    }

    constexpr bool      is_bayer_fcc( img::fourcc fcc ) noexcept {
        return  is_by8_fcc( fcc ) || 
                is_by16_fcc( fcc ) || 
                is_by10_packed_fcc( fcc ) || 
                is_by12_packed_fcc( fcc ) || 
                is_by12_fcc( fcc ) || 
                is_by10_fcc( fcc ) || 
                is_byfloat_fcc( fcc );
    }

    constexpr bool      is_bottom_up_fcc( img::fourcc fcc ) noexcept { return fcc == img::fourcc::BGR24 || fcc == img::fourcc::BGRA32 || fcc == img::fourcc::BGRA64; }
    constexpr bool      is_known_fcc( uint32_t fcc ) noexcept { return get_bits_per_pixel( fcc ) != 0; }
    constexpr bool      is_known_fcc( img::fourcc fcc ) noexcept    { return get_bits_per_pixel( fcc ) != 0; }
    constexpr bool      is_multi_plane_format( img::fourcc fcc ) noexcept { return calc_planar_fmt_count_of_planes( fcc ) > 1; }


    constexpr int       calc_planar_fmt_count_of_planes( img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::NV12:		                    return 2;
        case img::fourcc::YUV8PLANAR:	                    return 3;
        case img::fourcc::YUV16PLANAR:                    return 3;
        case img::fourcc::YUVF32PLANAR:                   return 3;
        case img::fourcc::POLARIZATION_ADI_PLANAR_MONO8:  return 4;
        case img::fourcc::POLARIZATION_ADI_PLANAR_MONO16: return 4;
        case img::fourcc::YV12:   return 3;
        case img::fourcc::I420:   return 3;
        default:
            return 1;
        }
    }


    namespace planar
    {
        struct planar_info
        {
            int plane_count = 1;
            struct plane_info
            {
                img::fourcc equiv_fcc = img::fourcc::FCC_NULL;
                int         bits_per_pixel = 8;
                float       scale_dim_x = 1;
                float       scale_dim_y = 1;
            } planes[4];
        };

        constexpr planar_info   get_fcc_info( fourcc fcc ) noexcept
        {
            switch( fcc )
            {
            case fourcc::YUV8PLANAR:    return planar_info{ 3, { { img::fourcc::MONO8, 8 },       { img::fourcc::RAW8, 8 },     { img::fourcc::RAW8, 8 } } };
            case fourcc::YUV16PLANAR:   return planar_info{ 3, { { img::fourcc::MONO16, 16 },     { img::fourcc::RAW16, 16 },   { img::fourcc::RAW16, 16 } } };
            case fourcc::YUVF32PLANAR:  return planar_info{ 3, { { img::fourcc::MONOFloat, 32 },  { img::fourcc::RAW32, 32 },   { img::fourcc::RAW32, 32 } } };

            case fourcc::NV12:          return planar_info{ 2, { { img::fourcc::MONO8, 8 },       { img::fourcc::RAW16, 16, 0.5f, 0.5f } } };
            case fourcc::YV12:          return planar_info{ 3, { { img::fourcc::MONO8, 8 },       { img::fourcc::RAW8, 8, 0.5f, 0.5f },     { img::fourcc::RAW8, 8, 0.5f, 0.5f } } };
            case fourcc::I420:          return planar_info{ 3, { { img::fourcc::MONO8, 8 },       { img::fourcc::RAW8, 8, 0.5f, 0.5f },     { img::fourcc::RAW8, 8, 0.5f, 0.5f } } };

            case fourcc::POLARIZATION_ADI_PLANAR_MONO8:
                return planar_info{ 4, { { img::fourcc::RAW8, 8 }, { img::fourcc::RAW8, 8 }, { img::fourcc::RAW8, 8 }, { img::fourcc::RAW8, 8 } } };
            case fourcc::POLARIZATION_ADI_PLANAR_MONO16:
                return planar_info{ 4, { { img::fourcc::RAW16, 16 }, { img::fourcc::RAW16, 16 }, { img::fourcc::RAW16, 16 }, { img::fourcc::RAW16, 16 } } };
            default:
                return {};
            }
        }

        constexpr planar_info::plane_info       get_fcc_info( fourcc fcc, int plane_index ) noexcept {
            return get_fcc_info( fcc ).planes[plane_index];
        }

        constexpr int       get_plane_count( fourcc fcc ) noexcept {
            return get_fcc_info( fcc ).plane_count;
        }
        constexpr float     get_plane_pitch_factor( fourcc fcc, int plane_index ) noexcept {
            const auto info = get_fcc_info( fcc ).planes[plane_index];
            return (info.scale_dim_x * info.bits_per_pixel) / 8;
        }
        constexpr int     get_plane_pitch_minimum( fourcc fcc, int dim_x, int plane_index ) noexcept {
            return static_cast<int>( get_plane_pitch_factor( fcc, plane_index ) * dim_x + 0.5f );
        }
        constexpr int     get_plane_size( fourcc fcc, img::dim dim, int plane_index ) noexcept {
            const auto info = get_fcc_info( fcc ).planes[plane_index];
            return static_cast<int>( get_plane_pitch_minimum( fcc, dim.cx, plane_index ) * dim.cy * info.scale_dim_y );
        }
        //constexpr int     get_plane_size( img::img_type type, int plane_index ) noexcept {
        //    return get_plane_size( type.fourcc_type(), type.dim, plane_index );
        //}
    }

    constexpr int       calc_minimum_pitch( img::fourcc fcc, int dim_x ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::YUV8PLANAR:	    return dim_x * 1;
        case img::fourcc::YUV16PLANAR:    return dim_x * 2;
        case img::fourcc::YUVF32PLANAR:   return dim_x * 4;

        case img::fourcc::POLARIZATION_ADI_PLANAR_MONO8:  return dim_x * 1;
        case img::fourcc::POLARIZATION_ADI_PLANAR_MONO16: return dim_x * 2;
        case img::fourcc::NV12:           return dim_x;
        case img::fourcc::YV12:           return dim_x;
        case img::fourcc::I420:           return dim_x;
        default:
            const int bpp = get_bits_per_pixel( fcc );
            assert( bpp != 0 );
            return (dim_x * bpp) / 8;
        }
    }
    constexpr int       calc_minimum_img_size( img::fourcc fcc, img::dim dim ) noexcept
    {
        assert( img::get_bits_per_pixel( fcc ) != 0 );
        switch( fcc )
        {
        case img::fourcc::NV12:	return (dim.cx * dim.cy) + (dim.cx * dim.cy / 4) * 2;       // u and v plane are sub-sampled 2x2, interleaved in 1 planes
        case img::fourcc::YV12: return (dim.cx * dim.cy) + (dim.cx * dim.cy / 4) * 2;       // u and v plane are sub-sampled 2x2
        case img::fourcc::I420: return (dim.cx * dim.cy) + (dim.cx * dim.cy / 4) * 2;       // u and v plane are sub-sampled 2x2
        default:
            const int bytes_per_line = calc_minimum_pitch( fcc, dim.cx );
            return bytes_per_line * dim.cy * calc_planar_fmt_count_of_planes( fcc );
        }
    }

    constexpr int       calc_img_size_from_bpp( img::dim dim, int bpp ) noexcept
    {
        const int bytes_per_line = (dim.cx * bpp) / 8;
        return bytes_per_line * dim.cy;
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
    constexpr bool is_fcc_in_fcclist( fourcc type, std::initializer_list<fourcc> typelist ) noexcept
    {
        for( auto&& e : typelist ) {
            if( type == e ) {
                return true;
            }
        }
        return false;
    }

#ifdef _MSC_VER
#else
#pragma GCC diagnostic pop  // re-enabled -Wswitch-enum
#endif

}
