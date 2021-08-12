

#pragma once

#include <cstdint>
#include "fcc1x_packed_to_fcc_internal.h"

namespace fcc1x_packed_internal
{
    inline uint16_t  calc_fcc12_to_fcc16( const void* src, int offset ) noexcept;
    inline uint16_t  calc_fcc12_mipi_to_fcc16( const void* src, int offset ) noexcept;
    inline uint16_t  calc_fcc12_packed_to_fcc16( const void* src, int offset ) noexcept;
    inline uint16_t  calc_fcc12_spacked_to_fcc16( const void* src, int offset ) noexcept;

    inline uint16_t  calc_fcc10_to_fcc16( const void* src, int offset ) noexcept;
    inline uint16_t  calc_fcc10_spacked_to_fcc16( const void* src, int offset ) noexcept;
    inline uint16_t  calc_fcc10_packed_mipi_to_fcc16( const void* src_line, int offset ) noexcept;

    inline uint16_t  calc_fcc16H12_to_fcc16( const void* src, int offset ) noexcept;
    inline uint16_t  calc_fcc16H12_to_fcc12( const void* src, int offset ) noexcept;

    inline uint16_t  calc_fcc12_mipi_to_fcc12( const void* src, int offset ) noexcept;

    inline uint32_t  calc_fcc12_mipi_to_fcc16L12_2x_pix( const void* src, int offset ) noexcept;
    inline auto      calc_fcc12_mipi_to_fcc16L12_2x_pix_str( const void* src, int offset ) noexcept;

    //////////////////////////////////////////////////////////////////////////
    //
    // convert BY12 (e.g. img::fourcc::GRBG12_PACKED) to BY16
    //  src == u8*, p == u16*
    //      p[0] = (src[0] << 8) | ((src[1] & 0x0F) << 4)
    //      p[1] = (src[2] << 8) | ((src[1] & 0xF0))
    //
    // convert BY12 MIPI (e.g. img::fourcc::GRBG12_MIPI_PACKED) to BY16
    //  src == u8*, p == u16*
    //      p[0] = (src[0] << 8) | ((src[2] & 0x0F) << 4)
    //      p[1] = (src[1] << 8) | ((src[2] & 0xF0))
    // 
    // convert BY12 stuff (e.g. img::fourcc::GRBG12_SPACKED) to BY16
    //  src == u8*, p == u16*
    //      p[0] = (src[0] & 0xFF) << 4 | (src[1] & 0x0F) << 12
    //      p[1] = (src[1] & 0xF0) << 0 | (src[2] & 0xFF) << 8
    // 


    inline uint16_t  calc_fcc12_packed_to_fcc16( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>( src ) + (offset / 2) * 3;
        if( offset % 2 == 0 ) {
            return uint16_t( uint32_t( src_ptr[0] ) << 8 | (uint32_t( src_ptr[1] & 0x0F ) << 4) );
        } else {
            return uint16_t( uint32_t( src_ptr[2] ) << 8 | (uint32_t( src_ptr[1] & 0xF0 ) << 0) );
        }
    }

    inline uint16_t  calc_fcc12_mipi_to_fcc16( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 2) * 3;
        if( offset % 2 == 0 ) {
            return uint16_t( uint32_t( src_ptr[0] ) << 8 | (uint32_t( src_ptr[2] & 0x0F ) << 4) );
        } else {
            return uint16_t( uint32_t( src_ptr[1] ) << 8 | (uint32_t( src_ptr[2] & 0xF0 ) << 0) );
        }
    }

    inline uint16_t  calc_fcc12_spacked_to_fcc16( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 2) * 3;
        if( offset % 2 == 0 ) {
            return uint16_t( uint32_t( src_ptr[0] & 0xFF ) << 4 | (uint32_t( src_ptr[1] & 0x0F ) << 12) );
        } else {
            return uint16_t( uint32_t( src_ptr[1] & 0xF0 ) << 0 | (uint32_t( src_ptr[2] & 0xFF ) << 8) );
        }
    }
    inline uint16_t  calc_fcc10_spacked_to_fcc16( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 4) * 5;
        switch( offset % 4 )        // idx 3 => 0,  33333333'33222222'22221111'11111100'00000000
        {
        case 0:     return (uint16_t( src_ptr[0] & 0b11111111 ) << (6 - 0)) | (uint16_t( src_ptr[1] & 0b00000011 ) << (8 + 6));
        case 1:     return (uint16_t( src_ptr[1] & 0b11111100 ) << (6 - 2)) | (uint16_t( src_ptr[2] & 0b00001111 ) << (6 + 6));
        case 2:     return (uint16_t( src_ptr[2] & 0b11110000 ) << (6 - 4)) | (uint16_t( src_ptr[3] & 0b00111111 ) << (4 + 6));
        case 3:     return (uint16_t( src_ptr[3] & 0b11000000 ) << (6 - 6)) | (uint16_t( src_ptr[4] & 0b11111111 ) << (2 + 6));
        };
        return 0;
    }
    inline uint16_t  calc_fcc10_packed_mipi_to_fcc16( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 4) * 5;
        switch( offset % 4 )        // 
        {
        case 0:     return (uint16_t( src_ptr[0] ) << 8) | (((src_ptr[4] & 0b00000011) >> 0) << 6);
        case 1:     return (uint16_t( src_ptr[1] ) << 8) | (((src_ptr[4] & 0b00001100) >> 2) << 6);
        case 2:     return (uint16_t( src_ptr[2] ) << 8) | (((src_ptr[4] & 0b00110000) >> 4) << 6);
        case 3:     return (uint16_t( src_ptr[3] ) << 8) | (((src_ptr[4] & 0b11000000) >> 6) << 6);
        };
        return 0;
    }

    inline uint16_t  calc_fcc12_to_fcc16( const void* src, int offset ) noexcept
    {
        return static_cast<const uint16_t*>(src)[offset] << 4;
    }

    inline uint16_t  calc_fcc10_to_fcc16( const void* src, int offset ) noexcept
    {
        return static_cast<const uint16_t*>(src)[offset] << 6;
    }

    inline uint16_t  calc_fcc16H12_to_fcc16( const void* src, int offset ) noexcept
    {
        return static_cast<const uint16_t*>(src)[offset];
    }

    inline uint16_t  calc_fcc16H12_to_fcc12( const void* src, int offset ) noexcept
    {
        return static_cast<const uint16_t*>(src)[offset] >> 4;
    }

    inline uint16_t  calc_fcc12_mipi_to_fcc12( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>( src ) + (offset / 2) * 3;
        if( offset % 2 == 0 ) {
            return (uint16_t( src_ptr[0] ) << 4 | (uint16_t( src_ptr[2] & 0x0F ) << 0));
        } else {
            return (uint16_t( src_ptr[1] ) << 4 | (uint16_t( src_ptr[2] & 0xF0 ) >> 4));
        }
    }

    inline uint32_t  calc_fcc12_mipi_to_fcc16_2x_pix( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 2) * 3;
        uint32_t val = 0;
        val |= (uint32_t( src_ptr[0] ) << 8 | (uint32_t( src_ptr[2] & 0x0F ) << 4)) << 0;
        val |= (uint32_t( src_ptr[1] ) << 8 | (uint32_t( src_ptr[2] & 0xF0 ) << 0)) << 16;
        return val;
    }

    inline uint32_t  calc_fcc12_mipi_to_fcc16L12_2x_pix( const void* src, int offset) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 2) * 3;
        uint32_t val = 0;
        val |= (uint32_t(src_ptr[0]) << 4 | (uint32_t(src_ptr[2] & 0x0F) << 0));
        val |= (uint32_t(src_ptr[1]) << 20 | (uint32_t(src_ptr[2] & 0xF0) << 12));
        return val;
    }

    inline auto      calc_fcc12_mipi_to_fcc12_2x_pix_str( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 2) * 3;

        uint16_t v0 = (uint32_t( src_ptr[0] ) << 4 | (uint32_t( src_ptr[2] & 0x0F ) << 0));
        uint16_t v1 = (uint32_t( src_ptr[1] ) << 4 | (uint32_t( src_ptr[2] & 0xF0 ) >> 4));

        struct tmp {
            uint16_t v0;
            uint16_t v1;
        };

        return tmp{ v0, v1 };
    }

}
