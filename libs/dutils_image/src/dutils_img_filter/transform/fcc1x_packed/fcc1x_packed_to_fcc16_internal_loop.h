

#pragma once

#include <cstdint>
#include "fcc1x_packed_to_fcc_internal.h"
#include "fcc1x_packed_to_fcc16_internal.h"

namespace fcc1x_packed_internal
{
    inline void     transform_fcc10_spacked_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept;

    inline void     transform_fcc12_packed_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept;
    inline void     transform_fcc12_mipi_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept;
    inline void     transform_fcc12_spacked_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept;

    inline void transform_fcc12_packed_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 2 )
        {
            const uint8_t* src_ptr = src_line + (x / 2) * 3;

            uint32_t val0 = 0;
            val0 |= (uint32_t( src_ptr[0] ) << 8 | (uint32_t( src_ptr[1] & 0x0F ) << 4));
            val0 |= (uint32_t( src_ptr[2] ) << 8 | (uint32_t( src_ptr[1] & 0xF0 ) << 0)) << 16;

            *(uint32_t*)(&dst_line[x + 0]) = val0;
        }
    }

    inline void transform_fcc12_mipi_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 2 )
        {
            uint32_t val0 = calc_fcc12_mipi_to_fcc16_2x_pix( src_line, x );
            *(uint32_t*)(&dst_line[x + 0]) = val0;
        }
    }

    inline void transform_fcc12_spacked_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 2 )
        {
            const uint8_t* src_ptr = src_line + (x / 2) * 3;
            
            uint32_t val = 0;
            val |= ((uint32_t( src_ptr[0] & 0xFF ) << 4) | (uint32_t( src_ptr[1] & 0x0F ) << 12)) << 0;
            val |= ((uint32_t( src_ptr[1] & 0xF0 ) << 0) | (uint32_t( src_ptr[2] & 0xFF ) << 8)) << 16;
            
            *(uint32_t*)(&dst_line[x + 0]) = val;
        }
    }

    inline void transform_fcc10_spacked_to_fcc16_c_line( const uint8_t* src_line, uint16_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 4 )
        {
            //   33333333'33222222'22221111'11111100'00000000

            const uint8_t* src = src_line + (x / 4) * 5;

            const uint64_t in = *reinterpret_cast<const uint32_t*>(src);

            uint64_t val = 0;
            val |= ((in & 0x000003FF) << 6);
            val |= ((in & 0x000FFC00) << 12);
            val |= ((in & 0x3FF00000) << 18);
            val |= ((in & 0xC0000000) << 24);
            val |= (uint64_t( src[4] ) << 56);

            *reinterpret_cast<uint64_t*>(dst_line + x) = val;
        }
    }
}
