

#pragma once

#include <cstdint>
#include "fcc1x_packed_to_fcc_internal.h"
#include "fcc1x_packed_to_fcc8_internal.h"

namespace fcc1x_packed_internal
{
    inline void transform_fcc12_packed_to_fcc8_c_line( const uint8_t* src_line, uint8_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 2 )
        {
            dst_line[x + 0] = calc_fcc12_packed_to_fcc8( src_line, x + 0 );
			dst_line[x + 1] = calc_fcc12_packed_to_fcc8( src_line, x + 1 );
        }
    }

    inline void transform_fcc12_mipi_to_fcc8_c_line( const uint8_t* src_line, uint8_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 2 )
        {
            dst_line[x + 0] = calc_fcc12_mipi_to_fcc8( src_line, x + 0 );
            dst_line[x + 1] = calc_fcc12_mipi_to_fcc8( src_line, x + 1 );
        }
    }

    inline void transform_fcc12_spacked_to_fcc8_c_line( const uint8_t* src_line, uint8_t* dst_line, int width ) noexcept
    {
		for( int x = 0; x < width; x += 2 )
		{
			dst_line[x + 0] = calc_fcc12_spacked_to_fcc8( src_line, x + 0 );
			dst_line[x + 1] = calc_fcc12_spacked_to_fcc8( src_line, x + 1 );
		}
    }

    inline void transform_fcc10_spacked_to_fcc8_c_line( const uint8_t* src_line, uint8_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 4 )
        {
            dst_line[x + 0] = calc_fcc10_spacked_to_fcc8( src_line, x + 0 );
            dst_line[x + 1] = calc_fcc10_spacked_to_fcc8( src_line, x + 1 );
            dst_line[x + 2] = calc_fcc10_spacked_to_fcc8( src_line, x + 2 );
            dst_line[x + 3] = calc_fcc10_spacked_to_fcc8( src_line, x + 3 );
        }
    }

    inline void transform_fcc10_mipi_to_fcc8_c_line( const uint8_t* src_line, uint8_t* dst_line, int width ) noexcept
    {
        for( int x = 0; x < width; x += 4 )
        {
            dst_line[x + 0] = calc_fcc10_mipi_to_fcc8( src_line, x + 0 );
            dst_line[x + 1] = calc_fcc10_mipi_to_fcc8( src_line, x + 1 );
            dst_line[x + 2] = calc_fcc10_mipi_to_fcc8( src_line, x + 2 );
            dst_line[x + 3] = calc_fcc10_mipi_to_fcc8( src_line, x + 3 );
        }
    }
}
