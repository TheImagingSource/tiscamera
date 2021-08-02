
#pragma once

#include "transform_mono_to_bgr.h"

#include <dutils_img/pixel_structs.h>


namespace transform_mono_to_bgr_internal
{
    using namespace img::pixel_type;

    FORCEINLINE
    void    transform_MONO8_to_BGR24_c_line( int cnt, const uint8_t* src_line, BGR24* dst_line )
    {
        for( int x = 0; x < cnt; ++x )
        {
            const auto val = src_line[x];
            dst_line[x] = BGR24{ val, val, val };
        }
    }

    FORCEINLINE
    void    transform_MONO8_to_BGRA32_c_line( int cnt, const uint8_t* src_line, BGRA32* dst_line )
    {
        for( int x = 0; x < cnt; ++x )
        {
            const auto val = src_line[x];
            dst_line[x] = BGRA32{ val, val, val, 0xFF };
        }
    }

    FORCEINLINE
    void    transform_MONO8_to_BGRA64_c_line( int cnt, const uint8_t* src_line, BGRA64* dst_line )
    {
        for( int x = 0; x < cnt; ++x )
        {
            const uint16_t val = static_cast<uint16_t>( src_line[x] ) << 8;
            dst_line[x] = BGRA64{ val, val, val, 0xFFFF };
        }
    }

    FORCEINLINE
    void    transform_MONO16_to_BGR24_c_line( int cnt, const uint16_t* src_line, BGR24* dst_line )
    {
        for( int x = 0; x < cnt; ++x )
        {
            const auto val = static_cast<uint8_t>( src_line[x] >> 8 );
            dst_line[x] = BGR24{ val, val, val };
        }
    }

    FORCEINLINE
    void    transform_MONO16_to_BGRA32_c_line( int cnt, const uint16_t* src_line, BGRA32* dst_line )
    {
        for( int x = 0; x < cnt; ++x )
        {
            const auto val = static_cast<uint8_t>(src_line[x] >> 8);
            dst_line[x] = BGRA32{ val, val, val, 0xFF };
        }
    }

    FORCEINLINE
    void    transform_MONO16_to_BGRA64_c_line( int cnt, const uint16_t* src_line, BGRA64* dst_line )
    {
        for( int x = 0; x < cnt; ++x )
        {
            const uint16_t val = src_line[x];
            dst_line[x] = BGRA64{ val, val, val, 0xFFFF };
        }
    }
}