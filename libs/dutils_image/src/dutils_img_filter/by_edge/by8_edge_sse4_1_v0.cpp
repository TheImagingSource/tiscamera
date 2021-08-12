
#include "by_edge.h"
#include "by_edge_internal.h"

#include "../simd_helper/use_simd_sse41.h"
#include "../simd_helper/sse_store_rgb_sse41.h"

#include "../../dutils_img_base/alignment_helper.h"


#include <cstring>

namespace
{
    using namespace by_edge_internal;
    using namespace img::by_transform::by_pattern_alg;

    using namespace simd::sse;

    struct alg_context_sse 
    {
        __m128i         clr_mtx[9];

        bool use_color_matrix;
        bool use_avg_green;
    };

    using alg_context = alg_context_sse;


template<class TOutStruct,bool use_nt_stores=false>
void    store( const line_data& lines, int x, const __m128i& r, const __m128i& g, const __m128i& b )
{
    static_assert(sizeof( TOutStruct ) != 1024, "Invalid parameter TOutStruct");
}

template<>
FORCEINLINE void    store<BGRA32,false>( const line_data& lines, int x, const __m128i& r, const __m128i& g, const __m128i& b )
{
    __m128i res0, res1, res2, res3;
    simd::sse::storage::rgb_SoA_epi8_to_rgb32_lo( r, g, b, res0, res1 );
    simd::sse::storage::rgb_SoA_epi8_to_rgb32_hi( r, g, b, res2, res3 );

    auto* p_out_line = reinterpret_cast<BGRA32*>(lines.out_line);
    store_u( p_out_line + x + 0, res0 );
    store_u( p_out_line + x + 4, res1 );
    store_u( p_out_line + x + 8, res2 );
    store_u( p_out_line + x + 12, res3 );
}

template<>
FORCEINLINE void    store<BGRA32,true>( const line_data& lines, int x, const __m128i& r, const __m128i& g, const __m128i& b )
{
    __m128i res0, res1, res2, res3;
    simd::sse::storage::rgb_SoA_epi8_to_rgb32_lo( r, g, b, res0, res1 );
    simd::sse::storage::rgb_SoA_epi8_to_rgb32_hi( r, g, b, res2, res3 );

    auto* p_out_line = reinterpret_cast<BGRA32*>(lines.out_line);
    store_s( p_out_line + x + 0, res0 );
    store_s( p_out_line + x + 4, res1 );
    store_s( p_out_line + x + 8, res2 );
    store_s( p_out_line + x + 12, res3 );
}

template<>
FORCEINLINE void    store<BGR24,false>( const line_data& lines, int x, const __m128i& r, const __m128i& g, const __m128i& b )
{
    __m128i res0, res1, res2;
    simd::sse::storage::rgb_SoA_epi8_to_rgb24_full( r, g, b, res0, res1, res2 );

    uint8_t* p_out = reinterpret_cast<uint8_t*>((reinterpret_cast<BGR24*>(lines.out_line) + x));
    store_u( p_out +  0, res0 );
    store_u( p_out + 16, res1 );
    store_u( p_out + 32, res2 );
}


template<>
FORCEINLINE void    store<BGR24, true>( const line_data& lines, int x, const __m128i& r, const __m128i& g, const __m128i& b )
{
    store<BGR24,false>( lines, x, r, g, b );
}


static const __m128i mask_0x00FF = INIT_M128i_SET1_EPU16( 0x00FF );
static const __m128i mask_0xFF00 = INIT_M128i_SET1_EPU16( 0xFF00 );


template<int base_index>
FORCEINLINE __m128i     apply_color_matrix_sse_chn_epu16( const alg_context& ctx, __m128i r, __m128i g, __m128i b )
{
    auto t0_lo = _mm_mullo_epi16( r, ctx.clr_mtx[base_index + 0] );
    auto t1_lo = _mm_mullo_epi16( g, ctx.clr_mtx[base_index + 1] );
    auto t2_lo = _mm_mullo_epi16( b, ctx.clr_mtx[base_index + 2] );

    auto sum = _mm_add_epi16( _mm_add_epi16( t0_lo, t1_lo ), t2_lo );
    auto val = _mm_srai_epi16( sum, 6 );

    val = _mm_max_epi16( val, _mm_setzero_si128() );        // saturate values < 0 to 0
    return val;
}

template<int base_index>
FORCEINLINE __m128i     apply_color_matrix_sse_chn( const alg_context& ctx, __m128i r, __m128i g, __m128i b )
{
    auto lo_r = _mm_cvtepu8_epi16( r );
    auto lo_g = _mm_cvtepu8_epi16( g );
    auto lo_b = _mm_cvtepu8_epi16( b );

    auto hi_r = _mm_unpackhi_epi8( r, _mm_setzero_si128() );
    auto hi_g = _mm_unpackhi_epi8( g, _mm_setzero_si128() );
    auto hi_b = _mm_unpackhi_epi8( b, _mm_setzero_si128() );

    auto val_lo = apply_color_matrix_sse_chn_epu16<base_index>( ctx, lo_r, lo_g, lo_b );
    auto val_hi = apply_color_matrix_sse_chn_epu16<base_index>( ctx, hi_r, hi_g, hi_b );

    auto res = _mm_packus_epi16( val_lo, val_hi );
    return res;
}

FORCEINLINE void        apply_color_matrix( const alg_context& ctx, __m128i& r, __m128i& g, __m128i& b )
{
    auto in_r = r;
    auto in_g = g;
    auto in_b = b;

    r = apply_color_matrix_sse_chn<0>( ctx, in_r, in_g, in_b );    // r
    g = apply_color_matrix_sse_chn<3>( ctx, in_r, in_g, in_b );    // r
    b = apply_color_matrix_sse_chn<6>( ctx, in_r, in_g, in_b );    // r
}

FORCEINLINE __m128i calc_x_from_xg_line( __m128i cur_p0, __m128i cur_p2 )
{
    // line= ...g3x2g1x0
    auto avg_line = _mm_avg_epu8( cur_p0, cur_p2 );                    // epu8 = ...,[g3+5],[x2+4],[g1+3],[x0+2]
    auto tmp2 = _mm_slli_epi16( cur_p2, 8 );

    return _mm_blendv_epi8( tmp2, avg_line, mask_0x00FF );
}

FORCEINLINE __m128i calc_x_from_gx_line( __m128i cur_p0, __m128i cur_p2 )
{
    // line= ...x3g2x1g0
    auto avg_line = _mm_avg_epu8( cur_p0, cur_p2 );                     // epu8 = ...,[x3+5],[g2+4],[x1+3],[g0+2]
    auto tmp2 = _mm_srli_epi16( cur_p0, 8 );                            // epu8 = ...,[  00],[  x3],[  00],[  x1]

    return _mm_blendv_epi8( tmp2, avg_line, mask_0xFF00 );
}

FORCEINLINE __m128i calc_y_from_xg_line( __m128i prv_p0, __m128i prv_p2, __m128i nxt_p0, const __m128i& nxt_p2 )
{
    // prv = ...y5g4y3g2y1g0
    auto prv_avg_line = _mm_avg_epu8( prv_p0, prv_p2 );             // epu8[] =  ..., avg(y3, y5), avg(g2,g4), avg(y1,y3), avg(g2,g0)
    auto nxt_avg_line = _mm_avg_epu8( nxt_p0, nxt_p2 );
    auto tmp1 = _mm_avg_epu8( prv_avg_line, nxt_avg_line );         // epu8[] = ..., avg(prv.y3,nxt.y3), avg(prv.g2,nxt.g2), avg(prv.y1,nxt.y1), avg(prv.g0,nxt.g0)

    auto tmp0 = _mm_avg_epu8( nxt_p0, prv_p0 );                     // epu8[] = ..., avg(prv.y3,nxt.y3), avg(prv.g2,nxt.g2), avg(prv.y1,nxt.y1), avg(prv.g0,nxt.g0)
    auto tmp2 = _mm_srli_epi16( tmp0, 8 );

    return _mm_blendv_epi8( tmp2, tmp1, mask_0xFF00 );
}

FORCEINLINE __m128i calc_y_from_gx_line( __m128i prv_p0, __m128i prv_p2, __m128i nxt_p0, const __m128i& nxt_p2 )
{
    // prv = y3g3y2g1y0
    auto prv_avg_line = _mm_avg_epu8( prv_p0, prv_p2 );             // epu8[] =  ..., avg(g3, g5), avg(y2,y4), avg(g1,g3), avg(y2,y0)
    auto nxt_avg_line = _mm_avg_epu8( nxt_p0, nxt_p2 );
    auto tmp1 = _mm_avg_epu8( prv_avg_line, nxt_avg_line );         // 

    auto tmp0 = _mm_avg_epu8( nxt_p2, prv_p2 );                     // epu8[] = ..., __, avg(prv.y2,nxt.y2), __, avg(prv.y0,nxt.y0)
    auto tmp2 = _mm_slli_epi16( tmp0, 8 );

    return _mm_blendv_epi8( tmp2, tmp1, mask_0x00FF );
}

FORCEINLINE __m128i calc_edge_g( __m128i cur_g_p0, __m128i cur_g_p2, __m128i prv_g, const __m128i& nxt_g )
{
    auto sum_lr = _mm_avg_epu8( cur_g_p0, cur_g_p2 );
    auto sum_ab = _mm_avg_epu8( prv_g, nxt_g );
    auto sum_al = _mm_avg_epu8( sum_lr, sum_ab );           // sum(prv[0],nxt[0],cur[-1],cur[+1]) / 4

    auto dif_lr = _mm_abs_epi16( _mm_sub_epi16( cur_g_p0, cur_g_p2 ) );
    auto dif_ab = _mm_abs_epi16( _mm_sub_epi16( prv_g, nxt_g ) );

    auto cmp_lt = _mm_cmplt_epi16( dif_lr, dif_ab );        // epi16[] = ..., (dif_lr[x] <  dif_ab[x] ? 0xFFFF : 0x0000)
    auto cmp_eq = _mm_cmpeq_epi16( dif_lr, dif_ab );        // epi16[] = ..., (dif_lr[x] == dif_ab[x] ? 0xFFFF : 0x0000)

    auto tmp0 = _mm_blendv_epi8( sum_ab, sum_lr, cmp_lt );  // epi16[] = ..., (dif_lr[x] <  dif_ab[x] ? sum_ab : sum_lr)
    auto tmp1 = _mm_blendv_epi8( tmp0, sum_al, cmp_eq );
    
    return tmp1;
}

FORCEINLINE __m128i calc_g_from_xg_line( __m128i prv_p2, __m128i cur_p0, __m128i cur_p2, const __m128i& nxt_p2 )
{
    // prv/nxt epu16[] = ??gx
    // cur     epu16[] = gx??

    auto cur_g_p0 = _mm_srli_epi16( cur_p0, 8 );     // epu16[x] = 00gx
    auto cur_g_p2 = _mm_srli_epi16( cur_p2, 8 );

    auto prv_g = _mm_and_si128( prv_p2, mask_0x00FF );
    auto nxt_g = _mm_and_si128( nxt_p2, mask_0x00FF );

    auto tmp1 = calc_edge_g( cur_g_p0, cur_g_p2, prv_g, nxt_g );
    auto tmp2 = _mm_slli_epi16( tmp1, 8 );
    return _mm_or_si128( cur_g_p0, tmp2 );
}

FORCEINLINE __m128i calc_g_from_gx_line( __m128i prv, __m128i cur_p0, __m128i cur_p2, const __m128i& nxt )
{
    auto cur_g_p0 = _mm_and_si128( cur_p0, mask_0x00FF );     // epu16[x] = 00gx
    auto cur_g_p2 = _mm_and_si128( cur_p2, mask_0x00FF );

    auto prv_g = _mm_srli_epi16( prv, 8 );
    auto nxt_g = _mm_srli_epi16( nxt, 8 );

    auto tmp1 = calc_edge_g( cur_g_p0, cur_g_p2, prv_g, nxt_g );
    auto tmp2 = _mm_slli_epi16( cur_g_p2, 8 );

    return _mm_or_si128( tmp1, tmp2 );
}

FORCEINLINE __m128i calc_avgG_value( __m128i prv_g_p0, __m128i prv_g_p2, __m128i nxt_g_p0, const __m128i& nxt_g_p2, const __m128i& cur_g )
{
    static const __m128i mask_0x0007 = INIT_M128i_SET1_EPU16( 0x0007 );

    auto dif_lr = _mm_abs_epi16( _mm_sub_epi16( prv_g_p0, prv_g_p2 ) );
    auto dif_ab = _mm_abs_epi16( _mm_sub_epi16( prv_g_p0, nxt_g_p0 ) );


#if 1
    auto t0 = _mm_avg_epu8( prv_g_p0, prv_g_p2 );
    auto t1 = _mm_avg_epu8( nxt_g_p0, nxt_g_p2 );

    auto sum_all = _mm_avg_epu8( t0, t1 );
    sum_all = _mm_avg_epu8( sum_all, cur_g );
#else
    auto s0 = _mm_add_epi16( prv_g_p0, prv_g_p2 );
    auto s1 = _mm_add_epi16( nxt_g_p0, nxt_g_p2 );
    auto sum_all = _mm_add_epi16( s0, s1 );
    sum_all = _mm_add_epi16( sum_all, _mm_slli_epi16( cur_g, 2 ) );
    sum_all = _mm_srli_epi16( sum_all, 3 );
#endif

    /*
        int dH = abs( prv[-1] - prv[+1] );
        int dV = abs( prv[-1] - nxt[-1] );
        if( 0x07 > dH && 0x07 > dV ) {
            return (prv[-1] + prv[1] + nxt[-1] + nxt[1] + cur[0] * 4) / 8
        } else {
            return cur[0];
        }
    */

    auto diff_gt_7 = _mm_cmplt_epi16( dif_ab, mask_0x0007 );
    auto diff_lr_7 = _mm_cmplt_epi16( dif_lr, mask_0x0007 );

    auto cond_true = _mm_and_si128( diff_gt_7, diff_lr_7 );

    return _mm_blendv_epi8( cur_g, sum_all, cond_true );
}

FORCEINLINE __m128i calc_g_from_xg_line_avgG( __m128i prv_p0, __m128i prv_p2, __m128i cur_p0, const __m128i& cur_p2, const __m128i& nxt_p0, const __m128i& nxt_p2 )
{
    // prv/nxt epu16[] = ??gx
    // cur     epu16[] = gx??

    auto cur_g_p0 = _mm_srli_epi16( cur_p0, 8 );     // epu16[x] = 00gx
    auto cur_g_p2 = _mm_srli_epi16( cur_p2, 8 );

    auto prv_g_p0 = _mm_and_si128( prv_p0, mask_0x00FF );
    auto nxt_g_p0 = _mm_and_si128( nxt_p0, mask_0x00FF );

    auto prv_g_p2 = _mm_and_si128( prv_p2, mask_0x00FF );
    auto nxt_g_p2 = _mm_and_si128( nxt_p2, mask_0x00FF );

    auto tmp1 = calc_edge_g( cur_g_p0, cur_g_p2, prv_g_p2, nxt_g_p2 );
    auto tmp2 = _mm_slli_epi16( tmp1, 8 );

    auto g_even = calc_avgG_value( prv_g_p0, prv_g_p2, nxt_g_p0, nxt_g_p2, cur_g_p0 );

    auto res = _mm_or_si128( g_even, tmp2 );
    return res;
}

FORCEINLINE __m128i calc_g_from_gx_line_avgG( __m128i prv_p0, __m128i prv_p2, __m128i cur_p0, const __m128i& cur_p2, const __m128i& nxt_p0, const __m128i& nxt_p2 )
{
    // prv/nxt epu16[] = ??gx
    // cur     epu16[] = gx??

    auto cur_g_p0 = _mm_and_si128( cur_p0, mask_0x00FF );     // epu16[x] = 00gx
    auto cur_g_p2 = _mm_and_si128( cur_p2, mask_0x00FF );

    auto prv_g_p0 = _mm_srli_epi16( prv_p0, 8 );
    auto nxt_g_p0 = _mm_srli_epi16( nxt_p0, 8 );

    auto prv_g_p2 = _mm_srli_epi16( prv_p2, 8 );
    auto nxt_g_p2 = _mm_srli_epi16( nxt_p2, 8 );

    auto tmp1 = calc_edge_g( cur_g_p0, cur_g_p2, prv_g_p0, nxt_g_p0 );

    auto g_even = calc_avgG_value( prv_g_p0, prv_g_p2, nxt_g_p0, nxt_g_p2, cur_g_p2 );
    auto tmp2 = _mm_slli_epi16( g_even, 8 );

    auto res = _mm_or_si128( tmp1, tmp2 );
    return res;
}

template<bool use_avg_green,by_pattern pat>
FORCEINLINE void    conv_sse_reg( __m128i& r, __m128i& g, __m128i& b,
                const __m128i& prv_p0, const __m128i& cur_p0, const __m128i& nxt_p0, 
                const __m128i& prv_p2, const __m128i& cur_p2, const __m128i& nxt_p2 )
{
    __m128i x_chn, y_chn, g_chn;
    if constexpr( is_gx_line( pat ) )
    {
        x_chn = calc_x_from_gx_line( cur_p0, cur_p2 );
        y_chn = calc_y_from_gx_line( prv_p0, prv_p2, nxt_p0, nxt_p2 );

        if( use_avg_green ) {
            g_chn = calc_g_from_gx_line_avgG( prv_p0, prv_p2, cur_p0, cur_p2, nxt_p0, nxt_p2 );
        } else {
            g_chn = calc_g_from_gx_line( prv_p0, cur_p0, cur_p2, nxt_p0 );
        }
    } else {
        x_chn = calc_x_from_xg_line( cur_p0, cur_p2 );
        y_chn = calc_y_from_xg_line( prv_p0, prv_p2, nxt_p0, nxt_p2 );

        if( use_avg_green ) {
            g_chn = calc_g_from_xg_line_avgG( prv_p0, prv_p2, cur_p0, cur_p2, nxt_p0, nxt_p2 );
        } else {
            g_chn = calc_g_from_xg_line( prv_p2, cur_p0, cur_p2, nxt_p2 );
        }
    }

    g = g_chn;
    if constexpr( is_red_line( pat ) ) {
        r = x_chn;
        b = y_chn;
    } else {
        r = y_chn;
        b = x_chn;
    }
}

template<class TOut,by_pattern pat, bool use_mtx, bool use_avg_green,bool use_nt_stores_>
void	    conv_line( const alg_context& clr, const line_data& lines_, int dim_x )
{
    auto lines = lines_;
    
    constexpr auto nxt_pattern = next_pixel( pat );
    constexpr bool use_nt_store = use_nt_stores_;
    
    {
        auto prv_lo_ = load_si128u( lines.lines[0] + 0 );
        auto cur_lo_ = load_si128u( lines.lines[1] + 0 );
        auto nxt_lo_ = load_si128u( lines.lines[2] + 0 );

        auto prv_lo = _mm_slli_si128( prv_lo_, 1 ); // epu8[0] = 0, epu8[x], x e [1;15[ = prv_log_[x - 1]
        auto cur_lo = _mm_slli_si128( cur_lo_, 1 );
        auto nxt_lo = _mm_slli_si128( nxt_lo_, 1 );

        int x = 0;

        for( ; x < (dim_x - 32); x += 16 )
        {
            // reads [x + 16;x + 32[
            auto prv_hi = load_si128u( lines.lines[0] + x + 16 - 1 );
            auto cur_hi = load_si128u( lines.lines[1] + x + 16 - 1 );
            auto nxt_hi = load_si128u( lines.lines[2] + x + 16 - 1 );

            auto prv_p2 = _mm_alignr_epi8( prv_hi, prv_lo, 2 );
            auto cur_p2 = _mm_alignr_epi8( cur_hi, cur_lo, 2 );
            auto nxt_p2 = _mm_alignr_epi8( nxt_hi, nxt_lo, 2 );

            __m128i r, g, b;
            conv_sse_reg<use_avg_green, nxt_pattern>( r, g, b, prv_lo, cur_lo, nxt_lo, prv_p2, cur_p2, nxt_p2 );

            if( use_mtx ) {
                apply_color_matrix( clr, r, g, b );
            }

            store<TOut, use_nt_store>( lines, x, r, g, b );       // writes [x + 1; x + 1 + 16[

            prv_lo = prv_hi;
            cur_lo = cur_hi;
            nxt_lo = nxt_hi;
        }

        if( x < (dim_x - 16 - 2 + 1) )      // x  = dim_x - 
        {
            auto prv_p2 = load_si128u( lines.lines[0] + x + 2 - 1 );
            auto cur_p2 = load_si128u( lines.lines[1] + x + 2 - 1 );
            auto nxt_p2 = load_si128u( lines.lines[2] + x + 2 - 1 );

            __m128i r, g, b;
            conv_sse_reg<use_avg_green, nxt_pattern>( r, g, b, prv_lo, cur_lo, nxt_lo, prv_p2, cur_p2, nxt_p2 );

            if( use_mtx ) {
                apply_color_matrix( clr, r, g, b );
            }

            store<TOut, use_nt_store>( lines, x, r, g, b );       // writes [x + 1; x + 1 + 16[
        }
    }

    if( dim_x > 18 )
    {
        // writes pixel [dim_x - 16;dim_x[
        // reads pixel  [dim_x - 32;dim_x[
        auto prv_minus_16 = load_si128u( lines.lines[0] + dim_x - 16 );
        auto cur_minus_16 = load_si128u( lines.lines[1] + dim_x - 16 );
        auto nxt_minus_16 = load_si128u( lines.lines[2] + dim_x - 16 );

        auto prv_p0 = load_si128u( lines.lines[0] + dim_x - 18 );
        auto cur_p0 = load_si128u( lines.lines[1] + dim_x - 18 );
        auto nxt_p0 = load_si128u( lines.lines[2] + dim_x - 18 );

        __m128i r, g, b;
        conv_sse_reg<use_avg_green, pat>( r, g, b, prv_p0, cur_p0, nxt_p0, prv_minus_16, cur_minus_16, nxt_minus_16 );

        if( use_mtx ) {
            apply_color_matrix( clr, r, g, b );
        }

        // double last entry, to get the equivalent to pixel copy
        static const __m128i shuf_last_byte = INIT_M128i_REG( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15 );
        r = _mm_shuffle_epi8( r, shuf_last_byte );
        g = _mm_shuffle_epi8( g, shuf_last_byte );
        b = _mm_shuffle_epi8( b, shuf_last_byte );

        int write_index = dim_x - 16;

        store<TOut>( lines, write_index, r, g, b );
    }
}

template<class TOut,bool ... TboolParams>
void	convert_by8_to_rgb_edge_sse4_1_v2( by_pattern pattern, const line_data& lines, int dim_x, const alg_context& ctx )
{
    if( pattern == by_pattern::BG ) {
        conv_line<TOut, by_pattern::BG, TboolParams...>( ctx, lines, dim_x );
    } else if( pattern == by_pattern::GB ) {
        conv_line<TOut, by_pattern::GB, TboolParams...>( ctx, lines, dim_x );
    } else if( pattern == by_pattern::GR ) {
        conv_line<TOut, by_pattern::GR, TboolParams...>( ctx, lines, dim_x );
    } else if( pattern == by_pattern::RG ) {
        conv_line<TOut, by_pattern::RG, TboolParams...>( ctx, lines, dim_x );
    }

    uint8_t* out_line = reinterpret_cast<uint8_t*>(lines.out_line);
    memcpy( out_line, out_line + sizeof( TOut ), sizeof( TOut ) );
}

template<class TOutDataType>
static void transform_line( by_pattern pattern, const line_data& lines, int dim_x, const alg_context& ctx )
{
    if( simd::is_aligned_for_stream<16>( lines.out_line ) ) 
    {
        if( ctx.use_color_matrix ) {
            if( ctx.use_avg_green ) {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, true,true, true>( pattern, lines, dim_x, ctx );
            } else  {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, true,false, true>( pattern, lines, dim_x, ctx );
            }
        } else {
            if( ctx.use_avg_green ) {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, false,true, true>( pattern, lines, dim_x, ctx );
            } else  {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, false,false, true>( pattern, lines, dim_x, ctx );
            }
        }
    }
    else
    {
        if( ctx.use_color_matrix ) {
            if( ctx.use_avg_green ) {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, true,true, false>( pattern, lines, dim_x, ctx );
            } else  {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, true,false, false>( pattern, lines, dim_x, ctx );
            }
        } else {
            if( ctx.use_avg_green ) {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, false,true, false>( pattern, lines, dim_x, ctx );
            } else  {
                convert_by8_to_rgb_edge_sse4_1_v2<TOutDataType, false,false, false>( pattern, lines, dim_x, ctx );
            }
        }
    }
}


alg_context_sse     fill_context( const img_filter::transform::by_edge::options& in_opt )
{
    auto ctx = alg_context_sse{ {}, in_opt.use_color_matrix, in_opt.use_avg_green };
    for( int i = 0; i < 9; ++i )
    {
        ctx.clr_mtx[i] = _mm_set1_epi16( in_opt.color_mtx.fac[i] );
    }
    return ctx;
}

template<typename TRGBStr>
static void by_edge_image_loop_sse41( img::img_descriptor dst_, img::img_descriptor src, const img_filter::transform::by_edge::options& options )
{
    auto dst = flip_image_in_img_desc_if_allowed( dst_ );


    by_pattern pattern_cur = img::by_transform::convert_bayer_fcc_to_pattern( src.fourcc_type() );
    by_pattern pattern_nxt = img::by_transform::by_pattern_alg::next_line( pattern_cur );

    auto in_opt = fill_context( options );

    int dim_y = src.dim.cy;

    if( !(src.flags & img::img_descriptor::flags_no_wrap_beg) ) {
        transform_line<TRGBStr>( pattern_cur, init_src_param( 0, dst, src, +1, +1 ), src.dim.cx, in_opt );
    } else {
        transform_line<TRGBStr>( pattern_cur, init_src_param( 0, dst, src, -1, +1 ), src.dim.cx, in_opt );
    }
    int y = 1;
    for( ; y < (dim_y - 1); y += 2 )
    {
        transform_line<TRGBStr>( pattern_nxt, init_src_param( y + 0, dst, src, -1, +1 ), src.dim.cx, in_opt );
        transform_line<TRGBStr>( pattern_cur, init_src_param( y + 1, dst, src, -1, +1 ), src.dim.cx, in_opt );
    }

    if( !(src.flags & img::img_descriptor::flags_no_wrap_end) ) {
        transform_line<TRGBStr>( pattern_nxt, init_src_param( y, dst, src, -1, -1 ), src.dim.cx, in_opt );
    } else {
        transform_line<TRGBStr>( pattern_nxt, init_src_param( y, dst, src, -1, +1 ), src.dim.cx, in_opt );
    }
}

}

img_filter::transform::by_edge::function_type   img_filter::transform::by_edge::get_transform_by8_to_dst_sse41( img::img_type dst, img::img_type src )
{
    if( !img::is_by8_fcc( src.fourcc_type() ) || dst.dim != src.dim ) {
        return nullptr;
    }
    if( dst.dim.cx < 18 || dst.dim.cy < 2 ) {
        return nullptr;
    }

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGRA32: return &by_edge_image_loop_sse41<BGRA32>;
    case img::fourcc::BGR24: return &by_edge_image_loop_sse41<BGR24>;
    default:
        break;
    };

    return nullptr;
}
