

#pragma once

#include "../transform_base.h"

namespace img::fcc1x_packed
{
    enum class fccXX_pack_type
    {
        fcc10,          // 16-bit lowest 10 valid
        fcc10_mipi,     // 10-bit packed, 4 packed in 5 bytes, [p0_hi][p1_hi][p2_hi][p3_hi][p0_lo | p1_lo << 2 | ---]
        fcc10_spacked,  // 10-bit packed, 4 packed in 5 bytes

        fcc12,          // 16-bit lowest 12 valid
        fcc12_mipi,     // 12-bit packed, 2 packed in 3 bytes, [p0_hi][p1_hi][p0_lo | p1_lo << 4]
        fcc12_packed,   // 12-bit packed, 2 packed in 3 bytes, [p0_hi][p0_lo | p1_lo << 4][p1_hi]
        fcc12_spacked,  // 12-bit packed, 2 packed in 3 bytes
       
        invalid,
    };

    struct pack_info
    {
        fccXX_pack_type                 pack_type = fccXX_pack_type::invalid;
        bool                            is_mono = false;
        img::by_transform::by_pattern   bayer_pattern = img::by_transform::by_pattern::RG;
    };
    
    constexpr pack_info         get_fcc1x_pack_info( const img::fourcc fcc ) noexcept;
    constexpr fccXX_pack_type   get_fcc1x_pack_type( const img::fourcc fcc ) noexcept;

    constexpr auto   get_fcc1x_pack_info( const img::fourcc fcc ) noexcept -> pack_info
    {

        switch( fcc )
        {
        case img::fourcc::BGGR10:               return pack_info{ fccXX_pack_type::fcc10, false, img::by_transform::by_pattern::BG };
        case img::fourcc::GBRG10:               return pack_info{ fccXX_pack_type::fcc10, false, img::by_transform::by_pattern::GB };
        case img::fourcc::RGGB10:               return pack_info{ fccXX_pack_type::fcc10, false, img::by_transform::by_pattern::RG };
        case img::fourcc::GRBG10:               return pack_info{ fccXX_pack_type::fcc10, false, img::by_transform::by_pattern::GR };

        case img::fourcc::MONO10:               return pack_info{ fccXX_pack_type::fcc10, true };

        case img::fourcc::BGGR10_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc10_mipi, false, img::by_transform::by_pattern::BG };
        case img::fourcc::GBRG10_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc10_mipi, false, img::by_transform::by_pattern::GB };
        case img::fourcc::RGGB10_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc10_mipi, false, img::by_transform::by_pattern::RG };
        case img::fourcc::GRBG10_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc10_mipi, false, img::by_transform::by_pattern::GR };

        case img::fourcc::MONO10_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc10_mipi, true };

        case img::fourcc::BGGR10_SPACKED:       return pack_info{ fccXX_pack_type::fcc10_spacked, false, img::by_transform::by_pattern::BG };
        case img::fourcc::GBRG10_SPACKED:       return pack_info{ fccXX_pack_type::fcc10_spacked, false, img::by_transform::by_pattern::GB };
        case img::fourcc::RGGB10_SPACKED:       return pack_info{ fccXX_pack_type::fcc10_spacked, false, img::by_transform::by_pattern::RG };
        case img::fourcc::GRBG10_SPACKED:       return pack_info{ fccXX_pack_type::fcc10_spacked, false, img::by_transform::by_pattern::GR };

        case img::fourcc::MONO10_SPACKED:        return pack_info{ fccXX_pack_type::fcc10_spacked, true };

        case img::fourcc::BGGR12:               return pack_info{ fccXX_pack_type::fcc12, false, img::by_transform::by_pattern::BG };
        case img::fourcc::GBRG12:               return pack_info{ fccXX_pack_type::fcc12, false, img::by_transform::by_pattern::GB };
        case img::fourcc::RGGB12:               return pack_info{ fccXX_pack_type::fcc12, false, img::by_transform::by_pattern::RG };
        case img::fourcc::GRBG12:               return pack_info{ fccXX_pack_type::fcc12, false, img::by_transform::by_pattern::GR };

        case img::fourcc::MONO12:               return pack_info{ fccXX_pack_type::fcc12, true };

        case img::fourcc::BGGR12_PACKED:        return pack_info{ fccXX_pack_type::fcc12_packed, false, img::by_transform::by_pattern::BG };
        case img::fourcc::GBRG12_PACKED:        return pack_info{ fccXX_pack_type::fcc12_packed, false, img::by_transform::by_pattern::GB };
        case img::fourcc::RGGB12_PACKED:        return pack_info{ fccXX_pack_type::fcc12_packed, false, img::by_transform::by_pattern::RG };
        case img::fourcc::GRBG12_PACKED:        return pack_info{ fccXX_pack_type::fcc12_packed, false, img::by_transform::by_pattern::GR };

        case img::fourcc::MONO12_PACKED:        return pack_info{ fccXX_pack_type::fcc12_packed, true };

        case img::fourcc::BGGR12_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc12_mipi, false, img::by_transform::by_pattern::BG };
        case img::fourcc::GBRG12_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc12_mipi, false, img::by_transform::by_pattern::GB };
        case img::fourcc::RGGB12_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc12_mipi, false, img::by_transform::by_pattern::RG };
        case img::fourcc::GRBG12_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc12_mipi, false, img::by_transform::by_pattern::GR };

        case img::fourcc::MONO12_MIPI_PACKED:   return pack_info{ fccXX_pack_type::fcc12_mipi, true };

        case img::fourcc::BGGR12_SPACKED:       return pack_info{ fccXX_pack_type::fcc12_spacked, false, img::by_transform::by_pattern::BG };
        case img::fourcc::GBRG12_SPACKED:       return pack_info{ fccXX_pack_type::fcc12_spacked, false, img::by_transform::by_pattern::GB };
        case img::fourcc::RGGB12_SPACKED:       return pack_info{ fccXX_pack_type::fcc12_spacked, false, img::by_transform::by_pattern::RG };
        case img::fourcc::GRBG12_SPACKED:       return pack_info{ fccXX_pack_type::fcc12_spacked, false, img::by_transform::by_pattern::GR };

        case img::fourcc::MONO12_SPACKED:       return pack_info{ fccXX_pack_type::fcc12_spacked, true };

        default:
            return pack_info{};
        };
    }

    constexpr fccXX_pack_type get_fcc1x_pack_type( const img::fourcc fcc ) noexcept
    {
        return get_fcc1x_pack_info( fcc ).pack_type;
    }

    constexpr bool  is_accepted_dst_fcc8( const img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::RAW8:
        case img::fourcc::MONO8:
        case img::fourcc::BGGR8:
        case img::fourcc::GBRG8:
        case img::fourcc::RGGB8:
        case img::fourcc::GRBG8:
            return true;
        default:
            // output format not accepted
            return false;
        }
    }

    constexpr bool  is_accepted_dst_fcc16( const img::fourcc fcc ) noexcept
    {
        switch( fcc )
        {
        case img::fourcc::RAW16:
        case img::fourcc::MONO16:
        case img::fourcc::BGGR16:
        case img::fourcc::GBRG16:
        case img::fourcc::RGGB16:
        case img::fourcc::GRBG16:
            return true;
        default:
            // output format not accepted
            return false;
        }
    }
}