

#pragma once

#include <cstdint>
#include "fcc1x_packed_to_fcc_internal.h"

namespace fcc1x_packed_internal
{
    inline uint8_t  calc_fcc16_to_fcc8( const void* src_line, int offset ) noexcept;

    inline uint8_t  calc_fcc12_to_fcc8( const void* src_line, int offset ) noexcept;
    inline uint8_t  calc_fcc12_packed_to_fcc8( const void* src, int offset ) noexcept;
    inline uint8_t  calc_fcc12_mipi_to_fcc8( const void* src, int offset ) noexcept;
    inline uint8_t  calc_fcc12_spacked_to_fcc8( const void* src, int offset ) noexcept;

    inline uint8_t  calc_fcc10_to_fcc8( const void* src_line, int offset ) noexcept;
    inline uint8_t  calc_fcc10_spacked_to_fcc8( const void* src_line, int offset ) noexcept;
    inline uint8_t  calc_fcc10_mipi_to_fcc8( const void* src_line, int offset ) noexcept;

    //////////////////////////////////////////////////////////////////////////
    //
    // convert BY12 (e.g. img::fourcc::GRBG12_PACKED) to BY16
    //  src == u8*, p == u16*
    //      p[0] = (src[0] << 8) | ((src[1] & 0x0F) << 4)
    //      p[1] = (src[2] << 8) | ((src[1] & 0xF0))
    //

    inline uint8_t  calc_fcc12_packed_to_fcc8( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>( src ) + (offset / 2) * 3;
        if( offset % 2 == 0 ) {
            return src_ptr[0];
        } else {
            return src_ptr[2];
        }
    }

	// convert BY12 MIPI (e.g. img::fourcc::GRBG12_MIPI_PACKED) to BY16
    //  src == u8*, p == u16*
    //      p[0] = (src[0] << 8) | ((src[2] & 0x0F) << 4)
    //      p[1] = (src[1] << 8) | ((src[2] & 0xF0))
    // 
    inline uint8_t  calc_fcc12_mipi_to_fcc8( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 2) * 3;
        if( offset % 2 == 0 ) {
            return src_ptr[0];
        } else {
            return src_ptr[1];
        }
    }

    // convert BY12 stuff (e.g. img::fourcc::GRBG12_SPACKED) to BY16
    //  src == u8*, p == u16*
    //      p[0] = (src[1] & 0x0F) << 4 | (src[0] & 0xF0) >> 4
    //      p[1] = src[2]
    // 
    inline uint8_t  calc_fcc12_spacked_to_fcc8( const void* src, int offset ) noexcept
    {
        const uint8_t* src_ptr = static_cast<const uint8_t*>(src) + (offset / 2) * 3;
        if( offset % 2 == 0 ) {
            return ((src_ptr[0] & 0xF0 ) >> 4) | ((src_ptr[1] & 0x0F) << 4);
        } else {
            return src_ptr[2];
        }
    }
    inline uint8_t  calc_fcc10_spacked_to_fcc8( const void* src_line, int offset ) noexcept
    {
        const uint8_t* src = static_cast<const uint8_t*>(src_line) + (offset / 4) * 5;
        switch( offset % 4 )        //   33333333'33222222'22221111'11111100'00000000
        {
        case 0:     return ((src[0] & 0b11111100 ) >> 2) | ((src[1] & 0b00000011 ) << (6));
        case 1:     return ((src[1] & 0b11110000 ) >> 4) | ((src[2] & 0b00001111 ) << (4));
        case 2:     return ((src[2] & 0b11000000 ) >> 6) | ((src[3] & 0b00111111 ) << (2));
        case 3:     return src[4];
        };
        return 0;
    }

    inline uint8_t  calc_fcc10_mipi_to_fcc8( const void* src_line, int offset ) noexcept
    {
        return (static_cast<const uint8_t*>(src_line) + (offset / 4) * 5)[offset % 4];
    }

    inline uint8_t  calc_fcc12_to_fcc8( const void* src_line, int offset ) noexcept
    {
        return (uint8_t)((static_cast<const uint16_t*>(src_line)[offset] & 0x0FFF) >> 4);
    }
    inline uint8_t  calc_fcc10_to_fcc8( const void* src_line, int offset ) noexcept
    {
        return (uint8_t)((static_cast<const uint16_t*>(src_line)[offset] & 0x03FF) >> 2);
    }
    inline uint8_t  calc_fcc16_to_fcc8( const void* src_line, int offset ) noexcept
    {
        return (uint8_t)(static_cast<const uint16_t*>(src_line)[offset] >> 8);
    }
}
