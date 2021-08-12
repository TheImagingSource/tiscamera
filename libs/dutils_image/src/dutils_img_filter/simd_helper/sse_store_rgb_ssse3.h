
#pragma once

#include "include_ssse3.h"

namespace simd {
namespace sse {
namespace storage
{
    void	rgb_SoA_epi8_to_rgb24_lo( __m128i r, __m128i g, __m128i b, __m128i& low );
    void	rgb_SoA_epi8_to_rgb24_lo( __m128i r, __m128i g, __m128i b, __m128i& low, __m128i& high );
    void	rgb_SoA_epi8_to_rgb24_full( __m128i r, __m128i g, __m128i b, __m128i& pix0, __m128i& pix1, __m128i& pix2 );

    void    rgb_SoA_epi8_to_rgb32_lo( __m128i r, __m128i g, __m128i b, __m128i& pixel03 );
    void    rgb_SoA_epi8_to_rgb32_lo( __m128i r, __m128i g, __m128i b, __m128i& pixel03, __m128i& pixel47 );
    void    rgb_SoA_epi8_to_rgb32_hi( __m128i r, __m128i g, __m128i b, __m128i& pixel03 );
    void    rgb_SoA_epi8_to_rgb32_hi( __m128i r, __m128i g, __m128i b, __m128i& pixel8B, __m128i& pixelCF );

    void	rgb_SoA_epu16_to_rgb24( __m128i r, __m128i g, __m128i b, __m128i& low, __m128i& high );
    void    rgb_SoA_epi16_to_rgb32( __m128i r, __m128i g, __m128i b, __m128i& pixel03, __m128i& pixel47 );

    void    rgb_SoA_epu16_to_rgb64( __m128i r, __m128i g, __m128i b, __m128i& pix0to1, __m128i& pix2to3, __m128i& pix4to5, __m128i& pix6to7 );
    void    rgb_SoA_epu16_to_rgb64_lo( __m128i r, __m128i g, __m128i b, __m128i& pix0to1, __m128i& pix2to3 );

    void    rgb_SoA_epu8_to_rgb32( __m128i r, __m128i g, __m128i b, __m128i& pixel03, __m128i& pixel47, __m128i& pixel8B, __m128i& pixelCF );


    namespace
    {
        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_bg = INIT_M128i_REG( 0x00, 0x01, 0x80, 0x02, 0x03, 0x80, 0x04, 0x05, 0x80, 0x06, 0x07, 0x80, 0x08, 0x09, 0x80, 0x0A );
        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_r = INIT_M128i_REG( 0x80, 0x80, 0x00, 0x80, 0x80, 0x01, 0x80, 0x80, 0x02, 0x80, 0x80, 0x03, 0x80, 0x80, 0x04, 0x80 );
        static const __m128i mov_hi_bg = INIT_M128i_REG( 0x0B, 0x80, 0x0C, 0x0D, 0x80, 0x0E, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );
        static const __m128i mov_hi_r = INIT_M128i_REG( 0x80, 0x05, 0x80, 0x80, 0x06, 0x80, 0x80, 0x07, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );

        static const __m128i shuff_bg_epu16_rgb24_lo = INIT_M128i_REG( 0x00, 0x08, 0x80, 0x01, 0x09, 0x80, 0x02, 0x0A, 0x80, 0x03, 0x0B, 0x80, 0x04, 0x0C, 0x80, 0x05 );
        static const __m128i shuff_bg_epu16_rgb24_hi = INIT_M128i_REG( 0x0D, 0x80, 0x06, 0x0E, 0x80, 0x07, 0x0F, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80 );
    }

    FORCEINLINE void	rgb_SoA_epi8_to_rgb24_lo( __m128i r, __m128i g, __m128i b, __m128i& low, __m128i& high )
    {
        __m128i bg = _mm_unpacklo_epi8( b, g );	                // G7G6G5G4G3G2G1G0B7B6B5B4B3B2B1B0 b

        __m128i low_bg = _mm_shuffle_epi8( bg, shuff_rgb_SoA_epi8_to_rgb24_bg );	// B5__G4B4__G3B3__G2B2__G1B1__G0B0 b
        __m128i low_r = _mm_shuffle_epi8( r, shuff_rgb_SoA_epi8_to_rgb24_r );		// __R4____R3____R2____R1____R0____ b

        low = _mm_or_si128( low_r, low_bg );					// B5R4G4B4R3G3B3R2G2B2R1G1B1R0G0B0 b

        __m128i high_bg = _mm_shuffle_epi8( bg, mov_hi_bg );	// __________________G7B7__G6B6__G5 b
        __m128i high_r = _mm_shuffle_epi8( r, mov_hi_r );	    // ________________R7____R6____R5__ b

        high = _mm_or_si128( high_r, high_bg );					// ________________R7G7B7R6G6B6R5G5 b
        // R7G7B7R6G6B6R5G5.B5R4G4B4R3G3B3R2G2B2R1G1B1R0G0B0
    }

    FORCEINLINE void	rgb_SoA_epi8_to_rgb24_lo( __m128i r, __m128i g, __m128i b, __m128i& low )
    {
        __m128i bg = _mm_unpacklo_epi8( b, g );	                // G7G6G5G4G3G2G1G0B7B6B5B4B3B2B1B0 b

        __m128i low_bg = _mm_shuffle_epi8( bg, shuff_rgb_SoA_epi8_to_rgb24_bg );	// B5__G4B4__G3B3__G2B2__G1B1__G0B0 b
        __m128i low_r = _mm_shuffle_epi8( r, shuff_rgb_SoA_epi8_to_rgb24_r );		// __R4____R3____R2____R1____R0____ b

        low = _mm_or_si128( low_r, low_bg );					// B5R4G4B4R3G3B3R2G2B2R1G1B1R0G0B0 b
    }

    FORCEINLINE void	rgb_SoA_epi8_to_rgb24_full( __m128i r, __m128i g, __m128i b, __m128i& pix0, __m128i& pix1, __m128i& pix2 )
    {
        __m128i bg_lo = _mm_unpacklo_epi8( b, g );	                // epi8[] = G7B7G6B6'G5B5G4B4'G3B3G2B2'G1B1G0B0
        __m128i bg_hi = _mm_unpackhi_epi8( b, g );	                // epu8[] = GfBfGeBe'GdBdGcBc'GbBbGaBa'G9B9G8B8
        __m128i bg_mi = _mm_alignr_epi8( bg_hi, bg_lo, 8 );        // epu8[] = GbBbGaBa'G9B9G8B8'G7B7G6B6'G5B5G4B4

        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_bg_0 = INIT_M128i_REG( 0x00, 0x01, 0x80, 0x02, 0x03, 0x80, 0x04, 0x05, 0x80, 0x06, 0x07, 0x80, 0x08, 0x09, 0x80, 0x0A );
        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_r0_0 = INIT_M128i_REG( 0x80, 0x80, 0x00, 0x80, 0x80, 0x01, 0x80, 0x80, 0x02, 0x80, 0x80, 0x03, 0x80, 0x80, 0x04, 0x80 );

        __m128i low_bg = _mm_shuffle_epi8( bg_lo, shuff_rgb_SoA_epi8_to_rgb24_bg_0 );	// B5__G4B4__G3B3__G2B2__G1B1__G0B0 b
        __m128i low_r0 = _mm_shuffle_epi8( r, shuff_rgb_SoA_epi8_to_rgb24_r0_0 );	    // __R4____R3____R2____R1____R0____ b

        pix0 = _mm_or_si128( low_r0, low_bg );					                        // epu8[] = B5R4G4B4R3G3B3R2G2B2R1G1B1R0G0B0

        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_bg_1 = INIT_M128i_REG( 0x03, 0x80, 0x04, 0x05, 0x80, 0x06, 0x07, 0x80, 0x08, 0x09, 0x80, 0x0A, 0x0B, 0x80, 0x0C, 0x0D );
        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_r0_1 = INIT_M128i_REG( 0x80, 0x05, 0x80, 0x80, 0x06, 0x80, 0x80, 0x07, 0x80, 0x80, 0x08, 0x80, 0x80, 0x09, 0x80, 0x80 );

        __m128i mid_bg = _mm_shuffle_epi8( bg_mi, shuff_rgb_SoA_epi8_to_rgb24_bg_1 );	// GaBa__G9B9__G8B8__G7B7__G6B6__G5 b
        __m128i mid_r0 = _mm_shuffle_epi8( r, shuff_rgb_SoA_epi8_to_rgb24_r0_1 );	    // ____R9____R8____R7____R6____R5__ b

        pix1 = _mm_or_si128( mid_r0, mid_bg );

        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_bg_2 = INIT_M128i_REG( 0x80, 0x06, 0x07, 0x80, 0x08, 0x09, 0x80, 0x0a, 0x0b, 0x80, 0x0c, 0x0d, 0x80, 0x0e, 0x0f, 0x80 );
        static const __m128i shuff_rgb_SoA_epi8_to_rgb24_r0_2 = INIT_M128i_REG( 0x0a, 0x80, 0x80, 0x0b, 0x80, 0x80, 0x0c, 0x80, 0x80, 0x0d, 0x80, 0x80, 0x0e, 0x80, 0x80, 0x0F );

        __m128i hig_bg = _mm_shuffle_epi8( bg_hi, shuff_rgb_SoA_epi8_to_rgb24_bg_2 );	// __GfBf__GeBe__GdBd__GcBc__GbBb__ b
        __m128i hig_r0 = _mm_shuffle_epi8( r, shuff_rgb_SoA_epi8_to_rgb24_r0_2 );	    // Rf____Re____Rd____Rc____Rb____Ra b

        pix2 = _mm_or_si128( hig_r0, hig_bg );
    }

    FORCEINLINE void	rgb_SoA_epu16_to_rgb24( __m128i r, __m128i g, __m128i b, __m128i& low, __m128i& high )
    {
        r = _mm_packus_epi16( r, _mm_setzero_si128() );         // ________________R7R6R5R4R3R2R1R0 b
        __m128i bg = _mm_packus_epi16( b, g );	                // G7G6G5G4G3G2G1G0B7B6B5B4B3B2B1B0 b

        __m128i low_bg = _mm_shuffle_epi8( bg, shuff_bg_epu16_rgb24_lo );	// B5__G4B4__G3B3__G2B2__G1B1__G0B0 b
        __m128i low_r = _mm_shuffle_epi8( r, shuff_rgb_SoA_epi8_to_rgb24_r );		// __R4____R3____R2____R1____R0____ b

        low = _mm_or_si128( low_r, low_bg );					// B5R4G4B4R3G3B3R2G2B2R1G1B1R0G0B0 b

        __m128i high_bg = _mm_shuffle_epi8( bg, shuff_bg_epu16_rgb24_hi );	// __________________G7B7__G6B6__G5 b
        __m128i high_r = _mm_shuffle_epi8( r, mov_hi_r );	    // ________________R7____R6____R5__ b

        high = _mm_or_si128( high_r, high_bg );					// ________________R7G7B7R6G6B6R5G5 b
                                                                // R7G7B7R6G6B6R5G5.B5R4G4B4R3G3B3R2G2B2R1G1B1R0G0B0
    }

    FORCEINLINE void rgb_SoA_epi8_to_rgb32_lo( __m128i r, __m128i g, __m128i b, __m128i& pixel03 )
    {
        // generate a register with epi8[16] = 0xFF
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

        __m128i bg = _mm_unpacklo_epi8( b, g );			// bg =		G7B7G6B6G5B5G4B4G3B3G2B2G1B1G0B0
        __m128i rf = _mm_unpacklo_epi8( r, full_ff );	// rf =	    FFR7FFR6FFR5FFT4FFR3FFR2FFR1FFR0

        pixel03 = _mm_unpacklo_epi16( bg, rf );		    // low =	FFR3G3B3FFR2G2B2FFR1G1B1FFR0G0B0
    }

    FORCEINLINE void rgb_SoA_epi8_to_rgb32_lo( __m128i r, __m128i g, __m128i b, __m128i& pixel03, __m128i& pixel47 )
    {
        // generate a register with epi8[16] = 0xFF
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

        __m128i bg = _mm_unpacklo_epi8( b, g );			// bg =		G7B7G6B6G5B5G4B4G3B3G2B2G1B1G0B0
        __m128i rf = _mm_unpacklo_epi8( r, full_ff );	// rf =	    FFR7FFR6FFR5FFT4FFR3FFR2FFR1FFR0

        pixel03 = _mm_unpacklo_epi16( bg, rf );		    // low =	FFR3G3B3FFR2G2B2FFR1G1B1FFR0G0B0
        pixel47 = _mm_unpackhi_epi16( bg, rf );
    }

    FORCEINLINE void rgb_SoA_epi8_to_rgb32_hi( __m128i r, __m128i g, __m128i b, __m128i& pixel03 )
    {
        // generate a register with epi8[16] = 0xFF
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

        __m128i bg = _mm_unpackhi_epi8( b, g );			// bg =		G7B7G6B6G5B5G4B4G3B3G2B2G1B1G0B0
        __m128i rf = _mm_unpackhi_epi8( r, full_ff );	// rf =	    FFR7FFR6FFR5FFT4FFR3FFR2FFR1FFR0

        pixel03 = _mm_unpacklo_epi16( bg, rf );		    // low =	FFR3G3B3FFR2G2B2FFR1G1B1FFR0G0B0
    }

    FORCEINLINE void rgb_SoA_epi8_to_rgb32_hi( __m128i r, __m128i g, __m128i b, __m128i& pixel8B, __m128i& pixelCF )
    {
        // generate a register with epi8[16] = 0xFF
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

        __m128i bg = _mm_unpackhi_epi8( b, g );			// bg =		G7B7G6B6G5B5G4B4G3B3G2B2G1B1G0B0
        __m128i rf = _mm_unpackhi_epi8( r, full_ff );	// rf =	    FFR7FFR6FFR5FFT4FFR3FFR2FFR1FFR0

        pixel8B = _mm_unpacklo_epi16( bg, rf );		    // low =	FFR3G3B3FFR2G2B2FFR1G1B1FFR0G0B0
        pixelCF = _mm_unpackhi_epi16( bg, rf );
    }

    FORCEINLINE
    void    rgb_SoA_epu16_to_rgb64( __m128i r, __m128i g, __m128i b, __m128i& pix0to1, __m128i& pix2to3, __m128i& pix4to5, __m128i& pix6to7 )
    {
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

        __m128i bg_hi = _mm_unpackhi_epi16( b, g );         // epi16, g7b7g6b6g5b5g4b4
        __m128i bg_lo = _mm_unpacklo_epi16( b, g );         // epi16, g3b3g2b2g1b1g0b0
        __m128i rFF_lo = _mm_unpacklo_epi16( r, full_ff );  // epi16, FFr3FFr2FFr1FFr0
        __m128i rFF_hi = _mm_unpackhi_epi16( r, full_ff );  // 

        pix0to1 = _mm_unpacklo_epi32( bg_lo, rFF_lo );      // epi16: FFr1g1b1FFr0g0b0
        pix2to3 = _mm_unpackhi_epi32( bg_lo, rFF_lo );
        pix4to5 = _mm_unpacklo_epi32( bg_hi, rFF_hi );
        pix6to7 = _mm_unpackhi_epi32( bg_hi, rFF_hi );
    }

    FORCEINLINE
    void    rgb_SoA_epu16_to_rgb64_lo( __m128i r, __m128i g, __m128i b, __m128i& pix0to1, __m128i& pix2to3 )
    {
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

        __m128i bg_lo = _mm_unpacklo_epi16( b, g );         // epi16, g3b3g2b2g1b1g0b0
        __m128i rFF_lo = _mm_unpacklo_epi16( r, full_ff );  // epi16, FFr3FFr2FFr1FFr0

        pix0to1 = _mm_unpacklo_epi32( bg_lo, rFF_lo );      // epi16: FFr1g1b1FFr0g0b0
        pix2to3 = _mm_unpackhi_epi32( bg_lo, rFF_lo );
    }


    // r = epi16[x] = 0x00Rx
    FORCEINLINE
    void rgb_SoA_epi16_to_rgb32( __m128i r, __m128i g, __m128i b, __m128i& pixel03, __m128i& pixel47 )
    {
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // epi8[x] = 0xFF
        __m128i reg_00ff = _mm_srli_epi16( full_ff, 8 );   // epi16[x] = 0x00FF

        __m128i br = _mm_packus_epi16( b, r );          // epi8[0;7] = Rx, epi8[8,15] = Bx, R7R6R5R4R3R2R1R0'B7B6B5B4B3B2B1B0
        __m128i gf = _mm_packus_epi16( g, reg_00ff );   // epi8[0;7] = Gx, epi8[8,15] = FF, FFFFFFFFFFFFFFFF'G7G6G5G4G3G2G1G0

        __m128i bg = _mm_unpacklo_epi8( br, gf );		// bg =		G7B7G6B6G5B5G4B4G3B3G2B2G1B1G0B0
        __m128i rf = _mm_unpackhi_epi8( br, gf );	    // rf =	    FFR7FFR6FFR5FFT4FFR3FFR2FFR1FFR0

        pixel03 = _mm_unpacklo_epi16( bg, rf );		    // low =	FFR3G3B3FFR2G2B2FFR1G1B1FFR0G0B0
        pixel47 = _mm_unpackhi_epi16( bg, rf );
    }


    FORCEINLINE
    void rgb_SoA_epu8_to_rgb32( __m128i r, __m128i g, __m128i b, __m128i& pixel03, __m128i& pixel47, __m128i& pixel8B, __m128i& pixelCF )
    {
        // b = epu8[x] = Bx
        __m128i full_ff = _mm_cmpeq_epi8( _mm_setzero_si128(), _mm_setzero_si128() );   // epi8[x] = 0xFF

        __m128i r_lo = _mm_unpacklo_epi8( r, full_ff );         // r_lo =   FFR7FFR6FFR5FFT4FFR3FFR2FFR1FFR0
        __m128i r_hi = _mm_unpackhi_epi8( r, full_ff );

        __m128i bg_lo = _mm_unpacklo_epi8( b, g );              // bg =		G7B7G6B6G5B5G4B4G3B3G2B2G1B1G0B0
        __m128i bg_hi = _mm_unpackhi_epi8( b, g );

        pixel03 = _mm_unpacklo_epi16( bg_lo, r_lo );            // low =	FFR3G3B3FFR2G2B2FFR1G1B1FFR0G0B0
        pixel47 = _mm_unpackhi_epi16( bg_lo, r_lo );
        pixel8B = _mm_unpacklo_epi16( bg_hi, r_hi );
        pixelCF = _mm_unpackhi_epi16( bg_hi, r_hi );
    }

} // storage
} // sse
} // simd