
#pragma once

#include "../include_ssse3.h"

namespace simd {
namespace sse {
    namespace mem {
        enum mem_access {
            unaligned,
            aligned,
            stream,
        };

        template<mem_access T>
        FORCEINLINE                 __m128i		load_si128( const void* addr ) = delete;
        template<>  FORCEINLINE     __m128i		load_si128<unaligned>( const void* addr )         { return _mm_loadu_si128( static_cast<const __m128i*>(addr) ); }
        template<>  FORCEINLINE     __m128i		load_si128<aligned>( const void* addr )           { return _mm_load_si128( static_cast<const __m128i*>(addr) ); }

        template<mem_access T>
        FORCEINLINE                 __m128		load_ps( const void* addr ) = delete;
        template<>  FORCEINLINE     __m128		load_ps<unaligned>( const void* addr )            { return _mm_loadu_ps( static_cast<const float*>(addr) ); }
        template<>  FORCEINLINE     __m128		load_ps<aligned>( const void* addr )              { return _mm_load_ps( static_cast<const float*>(addr) ); }


        FORCEINLINE                 __m128		load_ps4a( const void* addr )                     { return _mm_load_ps( static_cast<const float*>(addr) ); }
        FORCEINLINE                 __m128		load_ps4u( const void* addr )                     { return _mm_loadu_ps( static_cast<const float*>(addr) ); }

        FORCEINLINE                 __m128i     load_si128u( const void* addr )                   { return _mm_loadu_si128( static_cast<const __m128i*>(addr) ); }
        FORCEINLINE                 __m128i     load_si128a( const void* addr )                   { return _mm_load_si128( static_cast<const __m128i*>(addr) ); }

        FORCEINLINE                 __m128i     load_epi64u( const void* addr )                   { return _mm_loadl_epi64( static_cast<const __m128i*>(addr) ); }

        template<mem_access T>
        FORCEINLINE                 void		store( void* addr, __m128i val ) = delete;

        template<>  FORCEINLINE     void		store<unaligned>( void* addr, __m128i val ) { _mm_storeu_si128( static_cast<__m128i*>(addr), val ); }
        template<>  FORCEINLINE     void		store<aligned>( void* addr, __m128i val )   { _mm_store_si128( static_cast<__m128i*>(addr), val ); }
        template<>  FORCEINLINE     void		store<stream>( void* addr, __m128i val )    { _mm_stream_si128( static_cast<__m128i*>(addr), val ); }

        template<mem_access T>
        FORCEINLINE                 void		store( void* addr, __m128 val ) = delete;

        template<>  FORCEINLINE     void		store<unaligned>( void* addr, __m128 val )  { _mm_storeu_ps( static_cast<float*>(addr), val ); }
        template<>  FORCEINLINE     void		store<aligned>( void* addr, __m128 val )    { _mm_store_ps( static_cast<float*>(addr), val ); }
        template<>  FORCEINLINE     void		store<stream>( void* addr, __m128 val )     { _mm_stream_ps( static_cast<float*>(addr), val ); }

        FORCEINLINE                 void        store_epi64u( void* addr, __m128i val ) { _mm_storel_epi64( static_cast<__m128i*>(addr), val ); }

        FORCEINLINE                 void        store_u( void* addr, __m128 val )       { _mm_storeu_ps( static_cast<float*>(addr), val ); }
        FORCEINLINE                 void        store_u( void* addr, __m128i val )      { _mm_storeu_si128( static_cast<__m128i*>(addr), val ); }

        FORCEINLINE                 void        store_a( void* addr, __m128 val )       { _mm_store_ps( static_cast<float*>(addr), val ); }
        FORCEINLINE                 void        store_a( void* addr, __m128i val )      { _mm_store_si128( static_cast<__m128i*>(addr), val ); }

        FORCEINLINE                 void        store_s( void* addr, __m128 val )       { _mm_stream_ps( static_cast<float*>(addr), val ); }
        FORCEINLINE                 void        store_s( void* addr, __m128i val )      { _mm_stream_si128( static_cast<__m128i*>(addr), val ); }
    }

    using namespace mem;

    __m128i     clip_epi16_to_byte( __m128i value );
    __m128i     clip_epu16_to_byte( __m128i value );
    __m128      clip_ps_uint16( __m128 v );

    __m128i     _mm_mullo_epi32_sse2( const __m128i &a, const __m128i &b );
    __m128i     _mm_packus_epi32_sse2( __m128i t0, __m128i t1 );

    static const __m128i interpolate_epi16_mask_255 = INIT_M128i_SET1_EPU16( 255 );

    // rval.i16[i] = CLIP( value.i16[i], 0, 0xFF )
    FORCEINLINE __m128i clip_epi16_to_byte( __m128i value )
    {
        return _mm_min_epi16( _mm_max_epi16( value, _mm_setzero_si128() ), interpolate_epi16_mask_255 );
    }

    FORCEINLINE __m128i clip_epu16_to_byte( __m128i value )
    {
        return _mm_min_epi16( value, interpolate_epi16_mask_255 );
    }

    FORCEINLINE __m128i _mm_mullo_epi32_sse2( const __m128i &a, const __m128i &b )
    {
        __m128i tmp1 = _mm_mul_epu32( a, b );                                           /* mul 2,0*/
        __m128i tmp2 = _mm_mul_epu32( _mm_srli_si128( a, 4 ), _mm_srli_si128( b, 4 ) ); /* mul 3,1 */
        return _mm_unpacklo_epi32( _mm_shuffle_epi32( tmp1, _MM_SHUFFLE( 0, 0, 2, 0 ) ), _mm_shuffle_epi32( tmp2, _MM_SHUFFLE( 0, 0, 2, 0 ) ) ); /* shuffle results to [63..0] and pack */
    }


    FORCEINLINE __m128i	    _mm_packus_epi32_sse2( __m128i a, __m128i b )
    {
        const static __m128i val_32 = INIT_M128i_SET1_EPU32( 0x8000 );
        const static __m128i val_16 = INIT_M128i_SET1_EPU16( 0x8000 );

        a = _mm_sub_epi32( a, val_32 );
        b = _mm_sub_epi32( b, val_32 );
        a = _mm_packs_epi32( a, b );
        a = _mm_add_epi16( a, val_16 );
        return a;
    }
#if 0
    FORCEINLINE __m128i _mm_packus_epi32_sse2( __m128i t0, __m128i t1 ) // this seems to be buggy for values > 40000 ...
    {
        t0 = _mm_slli_epi32( t0, 16 );
        t0 = _mm_srai_epi32( t0, 16 );
        t1 = _mm_slli_epi32( t1, 16 );
        t1 = _mm_srai_epi32( t1, 16 );
        t0 = _mm_packs_epi32( t0, t1 );
        return t0;
    }
#endif

    FORCEINLINE __m128i _mm_cvtepu8_epi16_sse2( __m128i t )
    {
        return _mm_unpacklo_epi8( t, _mm_setzero_si128() );
    }

    namespace {
        static const __m128 max_uint16 = INIT_M128_SET1_PS( (float)256 * 256 - 1 );
    }

    FORCEINLINE __m128     clip_ps_uint16( __m128 v )
    {
        __m128 t0 = _mm_min_ps( v, max_uint16 );
        __m128 t1 = _mm_max_ps( t0, _mm_setzero_ps() );

        return t1;
    }

#if defined(_MSC_VER)
    FORCEINLINE     __m128  operator*( __m128 op1, __m128 op2 ) { return _mm_mul_ps( op1, op2 ); }
    FORCEINLINE     __m128  operator+( __m128 op1, __m128 op2 ) { return _mm_add_ps( op1, op2 ); }
    FORCEINLINE     __m128  operator-( __m128 op1, __m128 op2 ) { return _mm_sub_ps( op1, op2 ); }

    FORCEINLINE     __m128d  operator*( __m128d op1, __m128d op2 ) { return _mm_mul_pd( op1, op2 ); }
    FORCEINLINE     __m128d  operator+( __m128d op1, __m128d op2 ) { return _mm_add_pd( op1, op2 ); }
    FORCEINLINE     __m128d  operator-( __m128d op1, __m128d op2 ) { return _mm_sub_pd( op1, op2 ); }
#endif

    template<int pix_count>
    FORCEINLINE     __m128i  alignr_epi16( __m128i hi, __m128i lo )
    {
        static_assert(pix_count >= 0 && pix_count <= 8, "pix count must be in the range of [0;8]");
        if constexpr( pix_count == 0 ) {
            return lo;
        } else if constexpr( pix_count == 8 ) {
            return hi;
        } else {
            return _mm_alignr_epi8( hi, lo, pix_count * 2 );
        }
    }

    template<int pix_count>
    FORCEINLINE
    __m128       alignr_ps( __m128 hi, __m128 lo )
    {
        static_assert( pix_count >= 0 && pix_count <= 4, "pix count must be in the range of [0;4]" );
        return _mm_castsi128_ps( _mm_alignr_epi8( _mm_castps_si128( hi ), _mm_castps_si128( lo ), pix_count * 4 ) );
    }

    template<>
    FORCEINLINE
    __m128       alignr_ps<2>( __m128 hi, __m128 lo )
    {
        // return { hi[1], hi[0], lo[3], lo[2] }
        return _mm_shuffle_ps( lo, hi, _MM_SHUFFLE( 1, 0, 3, 2 ) );
    }
}
}
