
#pragma once

#include "../include_A8.h"

#ifdef _MSC_VER
#define CAST_REG_PARAM_U8_N( v, N )  ((uint64_t((v) & 0xFF)) << ((N) * 8))
#define CAST_REG_PARAM_U16_N( v, N )  ((uint64_t((v) & 0xFFFF)) << ((N) * 4))

#define INIT_Neon_U64( v0, v1, v2, v3, v4, v5, v6, v7 )     \
            CAST_REG_PARAM_U8_N(v0,0) | CAST_REG_PARAM_U8_N(v1,1) | CAST_REG_PARAM_U8_N(v2,2) | CAST_REG_PARAM_U8_N(v3,3) |   \
            CAST_REG_PARAM_U8_N( v4, 4 ) | CAST_REG_PARAM_U8_N( v5, 5 ) | CAST_REG_PARAM_U8_N( v6, 6 ) | CAST_REG_PARAM_U8_N( v7, 7 )

#define INIT_Neon_u8x8( v0, v1, v2, v3, v4, v5, v6, v7 )    \
        { INIT_Neon_U64( v0, v1, v2, v3, v4, v5, v6, v7 ) }

#define INIT_Neon_u16x4( v0, v1, v2, v3 )    \
        { CAST_REG_PARAM_U16_N( v0, 0 ) | CAST_REG_PARAM_U16_N( v1, 1 ) | CAST_REG_PARAM_U16_N( v2, 2 ) | CAST_REG_PARAM_U16_N( v3, 3 ) }


#define INIT_Neon_u8x16( v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF )    \
        { INIT_Neon_U64( v0, v1, v2, v3, v4, v5, v6, v7 ), INIT_Neon_U64( v8, v9, vA, vB, vC, vD, vE, vF ) }

#else

#define INIT_Neon_u8x8( v0, v1, v2, v3, v4, v5, v6, v7 )    \
        { v0, v1, v2, v3, v4, v5, v6, v7 }
#define INIT_Neon_u8x16( v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF )    \
        { v0, v1, v2, v3, v4, v5, v6, v7, v8, v9, vA, vB, vC, vD, vE, vF }

#define INIT_Neon_u16x4( v0, v1, v2, v3 )    \
        { v0, v1, v2, v3 }

#define INIT_Neon_u16x8( v0, v1, v2, v3, v4, v5, v6, v7 )    \
        { v0, v1, v2, v3, v4, v5, v6, v7 }

#define INIT_Neon_u16x8_set1( v0 )    \
        { v0, v0, v0, v0, v0, v0, v0, v0 }


#endif