
#pragma once

#include "../include_A64.h"
#include "sse_utils_A8.h"

namespace simd {
namespace neon {
    namespace mem
    {
        FORCEINLINE                 void        store_u( void* addr, float32x4_t val )  { vst1q_f32( static_cast<float*>(addr), val ); }
    }

    using namespace mem;

//
//#if defined(_MSC_VER)
//    FORCEINLINE     __m128  operator*( __m128 op1, __m128 op2 ) { return _mm_mul_ps( op1, op2 ); }
//    FORCEINLINE     __m128  operator+( __m128 op1, __m128 op2 ) { return _mm_add_ps( op1, op2 ); }
//    FORCEINLINE     __m128  operator-( __m128 op1, __m128 op2 ) { return _mm_sub_ps( op1, op2 ); }
//
//    FORCEINLINE     __m128d  operator*( __m128d op1, __m128d op2 ) { return _mm_mul_pd( op1, op2 ); }
//    FORCEINLINE     __m128d  operator+( __m128d op1, __m128d op2 ) { return _mm_add_pd( op1, op2 ); }
//    FORCEINLINE     __m128d  operator-( __m128d op1, __m128d op2 ) { return _mm_sub_pd( op1, op2 ); }
//#endif
//
}
}

// intrinsic NOTES:
// c = vzip_u16( a, b ) =>
//   uint16x4x2_t vzip_u16( uint16x4_t a, uint16x4_t b )
//      c.lo = {[0] = a[0], [1] = b[0], [2] = a[1], [3] = b[1] }
//      c.hi = {[0] = a[2], [1] = b[2], [2] = a[3], [3] = b[3] }
// c = vuzp_u16( a, b ) => 
//   uint16x4x2_t vuzp_u16( uint16x4_t a, uint16x4_t b )
//      c.lo = {[0] = a[0], [1] = a[2], [2] = b[0], [3] = b[1] }
//      c.hi = {[0] = a[1], [1] = a[3], [2] = b[1], [3] = b[3] }

namespace simd {
namespace neon {
namespace sse_helper {
        FORCEINLINE uint8x8_t   avg_epu8( uint8x8_t a, uint8x8_t b ) { return vrhadd_u8( a, b ); }

        FORCEINLINE void        store_u( void* addr, uint16x8_t a ) { vst1q_u16( static_cast<uint16_t*>(addr), a ); }
        FORCEINLINE void        store_u( void* addr, uint8x16_t a ) { vst1q_u8( static_cast<uint8_t*>(addr), a ); }
        FORCEINLINE void        store_u( void* addr, int16x8_t a ) { vst1q_s16( static_cast<int16_t*>(addr), a ); }
        FORCEINLINE void        store_u( void* addr, int8x16_t a ) { vst1q_s8( static_cast<int8_t*>(addr), a ); }

        FORCEINLINE uint8x16_t  load_si128u( const void* addr ) { return vld1q_u8( static_cast<const uint8_t*>(addr) ); }

        template<int pix_count>
        FORCEINLINE float32x4_t  alignr_ps( float32x4_t hi, float32x4_t lo ) { return vextq_f32( lo, hi, pix_count ); }

        FORCEINLINE uint16x8_t packus_epi32( int32x4_t a, int32x4_t b ) { return vcombine_u16( vqmovun_s32( a ), vqmovun_s32( b ) ); }
}

namespace helper
{
    FORCEINLINE uint16x4_t  vreinterpret_u16( uint8x8_t val ) noexcept { return vreinterpret_u16_u8( val ); }
    FORCEINLINE uint16x4_t  vreinterpret_u16( int16x4_t val ) noexcept { return vreinterpret_u16_s16( val ); }
    FORCEINLINE int16x4_t   vreinterpret_s16( uint8x8_t val ) noexcept { return vreinterpret_s16_u8( val ); }
    FORCEINLINE int16x4_t   vreinterpret_s16( uint16x4_t val ) noexcept { return vreinterpret_s16_u16( val ); }

    FORCEINLINE uint16x8_t  vreinterpret_u16( uint8x16_t val ) noexcept { return vreinterpretq_u16_u8( val ); }
    FORCEINLINE uint16x8_t  vreinterpret_u16( int16x8_t val ) noexcept { return vreinterpretq_u16_s16( val ); }
    FORCEINLINE int16x8_t   vreinterpret_s16( uint8x16_t val ) noexcept { return vreinterpretq_s16_u8( val ); }
    FORCEINLINE int16x8_t   vreinterpret_s16( uint16x8_t val ) noexcept { return vreinterpretq_s16_u16( val ); }

    FORCEINLINE uint32x4_t  vreinterpret_u32( uint8x16_t val ) noexcept { return vreinterpretq_u32_u8( val ); }
    FORCEINLINE uint32x4_t  vreinterpret_u32( uint16x8_t val ) noexcept { return vreinterpretq_u32_u16( val ); }
    FORCEINLINE uint32x4_t  vreinterpret_u32( uint32x4_t val ) noexcept = delete;   // most likely an error, so flag that
    FORCEINLINE uint32x4_t  vreinterpret_u32( int8x16_t val ) noexcept { return vreinterpretq_u32_s8( val ); }
    FORCEINLINE uint32x4_t  vreinterpret_u32( int16x8_t val ) noexcept { return vreinterpretq_u32_s16( val ); }
    FORCEINLINE uint32x4_t  vreinterpret_u32( int32x4_t val ) noexcept { return vreinterpretq_u32_s32( val ); }

    FORCEINLINE int32x4_t  vreinterpret_s32( uint8x16_t val ) noexcept { return vreinterpretq_s32_u8( val ); }
    FORCEINLINE int32x4_t  vreinterpret_s32( uint16x8_t val ) noexcept { return vreinterpretq_s32_u16( val ); }
    FORCEINLINE int32x4_t  vreinterpret_s32( uint32x4_t val ) noexcept { return vreinterpretq_s32_u32( val ); }
    FORCEINLINE int32x4_t  vreinterpret_s32( int8x16_t val ) noexcept { return vreinterpretq_s32_s8( val ); }
    FORCEINLINE int32x4_t  vreinterpret_s32( int16x8_t val ) noexcept { return vreinterpretq_s32_s16( val ); }
    FORCEINLINE int32x4_t  vreinterpret_s32( int32x4_t val ) noexcept = delete;  // most likely an error, so flag that

    FORCEINLINE uint8x8_t   vreinterpret_u8( uint16x4_t val ) noexcept { return vreinterpret_u8_u16( val ); }
    FORCEINLINE uint8x16_t  vreinterpret_u8( uint16x8_t val ) noexcept { return vreinterpretq_u8_u16( val ); }
    FORCEINLINE uint8x8_t   vreinterpret_u8( int16x4_t val ) noexcept { return vreinterpret_u8_s16( val ); }
    FORCEINLINE uint8x16_t  vreinterpret_u8( int16x8_t val ) noexcept { return vreinterpretq_u8_s16( val ); }

    FORCEINLINE uint8x8_t  vreinterpret_u8( uint32x2_t val ) noexcept { return vreinterpret_u8_u32( val ); }
    FORCEINLINE uint8x8_t  vreinterpret_u8( int32x2_t val ) noexcept { return vreinterpret_u8_s32( val ); }

    FORCEINLINE uint8x16_t  vreinterpret_u8( uint32x4_t val ) noexcept { return vreinterpretq_u8_u32( val ); }
    FORCEINLINE uint8x16_t  vreinterpret_u8( int32x4_t val ) noexcept { return vreinterpretq_u8_s32( val ); }

    FORCEINLINE int16x4_t   dvsub_s16( int16x4_t a, int16x4_t b ) noexcept { return vsub_s16( a, b ); }
    FORCEINLINE int16x4_t   dvsub_s16( uint16x4_t a, uint16x4_t b ) noexcept { return vsub_s16( vreinterpret_s16( a ), vreinterpret_s16( b ) ); }
    FORCEINLINE int16x8_t   dvsub_s16( int16x8_t a, int16x8_t b ) noexcept { return vsubq_s16( a, b ); }
    FORCEINLINE int16x8_t   dvsub_s16( uint16x8_t a, uint16x8_t b ) noexcept { return vsubq_s16( vreinterpret_s16( a ), vreinterpret_s16( b ) ); }
}
namespace storage
{
    using namespace simd::neon::helper;

    FORCEINLINE void    store_BGR32( void* dst, uint8x8_t r, uint8x8_t g, uint8x8_t b )
    {
        uint8x8x4_t t0 = { b, g, r, vdup_n_u8( 0xFF ) };

        vst4_u8( static_cast<uint8_t*>(dst), t0 );
    }
    FORCEINLINE void    store_BGR32( void* dst, uint8x16_t r, uint8x16_t g, uint8x16_t b )
    {
        uint8x16x4_t t0 = { b, g, r, vdupq_n_u8( 0xFF ) };

        vst4q_u8( static_cast<uint8_t*>(dst), t0 );
    }
    FORCEINLINE void    store_BGR24( void* dst, uint8x8_t r, uint8x8_t g, uint8x8_t b )
    {
        uint8x8x3_t t0 = { b, g, r };

        vst3_u8( static_cast<uint8_t*>(dst), t0 );
    }
    FORCEINLINE void    store_BGR24( void* dst, uint8x16_t r, uint8x16_t g, uint8x16_t b )
    {
        uint8x16x3_t t0 = { b, g, r };

        vst3q_u8( static_cast<uint8_t*>(dst), t0 );
    }

#if 0
    FORCEINLINE void    rgb_SoA_epi8_to_rgb32( uint8x8_t r, uint8x8_t g, uint8x8_t b, uint8x8_t& pix01, uint8x8_t& pix23, uint8x8_t& pix45, uint8x8_t& pix67 )
    {
        uint8x8_t reg_0xFF = INIT_Neon_u8x8( 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF );

        uint8x8x2_t bg = vzip_u8( b, g );            // u16[] = BG
        uint8x8x2_t rf = vzip_u8( r, reg_0xFF );

        auto tmp0 = vzip_u16( vreinterpret_u16( bg.val[0] ), vreinterpret_u16( rf.val[0] ) );
        pix01 = vreinterpret_u8( tmp0.val[0] );
        pix23 = vreinterpret_u8( tmp0.val[1] );
        auto tmp1 = vzip_u16( vreinterpret_u16( bg.val[1] ), vreinterpret_u16( rf.val[1] ) );
        pix45 = vreinterpret_u8( tmp1.val[0] );
        pix67 = vreinterpret_u8( tmp1.val[1] );
    }
    FORCEINLINE void    rgb_SoA_epi8_to_rgb32( uint8x16_t r, uint8x16_t g, uint8x16_t b, uint8x16_t& pix01, uint8x16_t& pix23, uint8x16_t& pix45, uint8x16_t& pix67 )
    {
        uint8x16_t reg_0xFF = INIT_Neon_u8x16( 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF );

        uint8x16x2_t bg = vzipq_u8( b, g );            // u16[] = BG
        uint8x16x2_t rf = vzipq_u8( r, reg_0xFF );

        auto tmp0 = vzipq_u16( vreinterpret_u16( bg.val[0] ), vreinterpret_u16( rf.val[0] ) );
        pix01 = vreinterpret_u8( tmp0.val[0] );
        pix23 = vreinterpret_u8( tmp0.val[1] );
        auto tmp1 = vzipq_u16( vreinterpret_u16( bg.val[1] ), vreinterpret_u16( rf.val[1] ) );
        pix45 = vreinterpret_u8( tmp1.val[0] );
        pix67 = vreinterpret_u8( tmp1.val[1] );
    }
#endif

}
}
}

// this is the A64 emulation

#if !defined DUTILS_ARCH_ARM_A64

FORCEINLINE uint8x16_t vqtbl1q_u8( uint8x16_t tbl, uint8x16_t v ) noexcept
{
    uint8x8x2_t tbl_ = { vget_low_u8( tbl ), vget_high_u8( tbl ) };
    return vcombine_u8(
        vtbl2_u8( tbl_, vget_low_u8( v ) ),
        vtbl2_u8( tbl_, vget_high_u8( v ) ) );
}


FORCEINLINE uint8x16_t vqtbl2q_u8( uint8x16x2_t tbl, uint8x16_t v ) noexcept
{
    uint8x8x4_t tbl_ = { vget_low_u8( tbl.val[0] ), vget_high_u8( tbl.val[0] ), vget_low_u8( tbl.val[1] ), vget_high_u8( tbl.val[1] ) };
    return vcombine_u8(
        vtbl4_u8( tbl_, vget_low_u8( v ) ),
        vtbl4_u8( tbl_, vget_high_u8( v ) )
    );
}

FORCEINLINE uint16x8_t vmovl_high_u8( uint8x16_t v0 ) noexcept { return vmovl_u8( vget_high_u8( v0 ) ); }
FORCEINLINE uint32x4_t vmovl_high_u16( uint16x8_t v0 ) noexcept { return vmovl_u16( vget_high_u16( v0 ) ); }

FORCEINLINE uint32x4_t vmull_high_n_u16( uint16x8_t v0, uint16_t n ) noexcept { return vmull_n_u16( vget_high_u16( v0 ), n ); }
FORCEINLINE uint32x4_t vmull_high_u16( uint16x8_t v0, uint16x8_t v1 ) noexcept { return vmull_u16( vget_high_u16( v0 ), vget_high_u16( v1 ) ); }

FORCEINLINE uint16x8_t vaddl_high_u8( uint8x16_t v0, uint8x16_t v1 ) noexcept { return vaddl_u8( vget_high_u8( v0 ), vget_high_u8( v1 ) ); }
FORCEINLINE uint16x8_t vaddw_high_u8( uint16x8_t v0, uint8x16_t v1 ) noexcept { return vaddw_u8( v0, vget_high_u8( v1 ) ); }
FORCEINLINE uint32x4_t vaddl_high_u16( uint16x8_t v0, uint16x8_t v1 ) noexcept { return vaddl_u16( vget_high_u16( v0 ), vget_high_u16( v1 ) ); }
FORCEINLINE uint32x4_t vaddw_high_u16( uint32x4_t v0, uint16x8_t v1 ) noexcept { return vaddw_u16( v0, vget_high_u16( v1 ) ); }

FORCEINLINE float32x4_t vdivq_f32( float32x4_t v0, float32x4_t v1 ) noexcept
{
    return float32x4_t{
        v0[0] / v1[0],
        v0[1] / v1[1],
        v0[2] / v1[2],
        v0[3] / v1[3]
    };
    //return vmulq_f32( v0, vrecpeq_f32( v1 ) );
}

#endif