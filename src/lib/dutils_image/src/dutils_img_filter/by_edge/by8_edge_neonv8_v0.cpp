

#include "by_edge_internal.h"

#include "../simd_helper/use_simd_A64.h"

#include <cstring>

namespace
{
    using namespace by_edge_internal;
    using namespace img::by_transform::by_pattern_alg;

    struct alg_context_sse 
    {
        int16x8_t       clr_mtx[9];

        bool use_color_matrix;
        bool use_avg_green;
    };

    using alg_context = alg_context_sse;

    using namespace simd::neon::helper;


template<class TOutStruct>
void    store( const line_data& lines, int x, uint8x16_t r, uint8x16_t g, uint8x16_t b ) = delete;

template<>
FORCEINLINE void    store<BGRA32>( const line_data& lines, int x, uint8x16_t r, uint8x16_t g, uint8x16_t b )
{
    uint8_t* p_out_line = static_cast<uint8_t*>(lines.out_line) + (x * sizeof( BGRA32 ));

    simd::neon::storage::store_BGR32( p_out_line, r, g, b );
/* Slow?
    uint8x16_t reg_0xFF = INIT_Neon_u8x16( 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF );

    uint8x16x4_t t0 = { b, g, r, reg_0xFF };

    vst4q_u8( reinterpret_cast<uint8_t*>(p_out_line) + 0, t0 );
*/
}

template<>
FORCEINLINE void    store<BGR24>( const line_data& lines, int x, uint8x16_t r, uint8x16_t g, uint8x16_t b )
{
    uint8_t* p_out_line = reinterpret_cast<uint8_t*>(lines.out_line) + x * sizeof( BGR24 );
    
    uint8x16x3_t t0 = { b, g, r };

    vst3q_u8( p_out_line + 0, t0 );
}

static const uint8x16_t mask_0x00FF = INIT_Neon_u8x16( 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00 );  // u8[0] = 0xFF, [1] = 0x00, 
static const uint8x16_t mask_0xFF00 = INIT_Neon_u8x16( 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF );

template<int base_index>
FORCEINLINE uint8x8_t     apply_color_matrix_sse_chn_epu16( const alg_context& ctx, uint16x8_t r, uint16x8_t g, uint16x8_t b )
{
    auto t0 = vmulq_s16( vreinterpretq_s16_u16( r ), ctx.clr_mtx[base_index + 0] );
    auto t1 = vmlaq_s16( t0, vreinterpretq_s16_u16( g ), ctx.clr_mtx[base_index + 1] );
    auto t2 = vmlaq_s16( t1, vreinterpretq_s16_u16( b ), ctx.clr_mtx[base_index + 2] );

    return vqrshrun_n_s16( t2, 6 );        // s16[] /= 64 and s16 ->u8 with saturation
}


template<int base_index>
FORCEINLINE uint8x16_t     apply_color_matrix_sse_chn( const alg_context& ctx, uint8x16_t r, uint8x16_t g, uint8x16_t b )
{
    auto r_u16_lo = vmovl_u8( vget_low_u8( r ) );
    auto g_u16_lo = vmovl_u8( vget_low_u8( g ) );
    auto b_u16_lo = vmovl_u8( vget_low_u8( b ) );
    uint8x8_t res_lo = apply_color_matrix_sse_chn_epu16<base_index>( ctx, r_u16_lo, g_u16_lo, b_u16_lo );
    uint8x8_t res_hi = apply_color_matrix_sse_chn_epu16<base_index>( ctx, vmovl_high_u8( r ), vmovl_high_u8( g ), vmovl_high_u8( b ) );

    return vcombine_u8( res_lo, res_hi );
}

FORCEINLINE void        apply_color_matrix( const alg_context& ctx, uint8x16_t& r, uint8x16_t& g, uint8x16_t& b )
{
    auto in_r = r;
    auto in_g = g;
    auto in_b = b;

    r = apply_color_matrix_sse_chn<0>( ctx, in_r, in_g, in_b );    // r
    g = apply_color_matrix_sse_chn<3>( ctx, in_r, in_g, in_b );    // r
    b = apply_color_matrix_sse_chn<6>( ctx, in_r, in_g, in_b );    // r
}


FORCEINLINE uint8x16_t calc_x_from_xg_line( uint8x16_t cur_p0, uint8x16_t cur_p2 ) // cur_p0=u8[], ...g3x2g1x0  cur_p2 = ...g5x4g3x2
{
    auto avg_line = vrhaddq_u8( cur_p0, cur_p2 );                    // epu8 = ...,[__],[x2+4],[__],[x0+2]
    auto tmp2 = vreinterpret_u8( vshlq_n_u16( vreinterpret_u16( cur_p2 ), 8 ) );                            // u8[] = [x4][00][x2][00]

    auto tmp3 = vbslq_u8( mask_0xFF00, tmp2, avg_line );            // 
    return tmp3;        // u8[] =  ...[x4][x2+4][x2][x0+2]
}

FORCEINLINE uint8x16_t calc_x_from_gx_line( uint8x16_t cur_p0, uint8x16_t cur_p2 )
{
    // line= ...x3g2x1g0
    auto avg_line = vrhaddq_u8( cur_p0, cur_p2 );                     // epu8 = ...,[x3+5],[g2+4],[x1+3],[g0+2]
    auto tmp2 = vreinterpret_u8( vshrq_n_u16( vreinterpret_u16( cur_p0 ), 8 ) );                            // epu8 = ...,[  00],[  x3],[  00],[  x1]

    auto tmp3 = vbslq_u8( mask_0x00FF, tmp2, avg_line );            // 
    return tmp3;        // u8[] =  ...[x2+4][x2][x1+3][x1]
}

FORCEINLINE uint8x16_t calc_y_from_xg_line( uint8x16_t prv_p0, uint8x16_t prv_p2, uint8x16_t nxt_p0, uint8x16_t nxt_p2 )
{
    // prv = ...y5g4y3g2y1g0
    auto prv_avg_line = vrhaddq_u8( prv_p0, prv_p2 );             // epu8[] =  ..., avg(y3, y5), avg(g2,g4), avg(y1,y3), avg(g2,g0)
    auto nxt_avg_line = vrhaddq_u8( nxt_p0, nxt_p2 );
    auto tmp1 = vrhaddq_u8( prv_avg_line, nxt_avg_line );         // epu8[] = ..., avg(prv.y3,nxt.y3), avg(prv.g2,nxt.g2), avg(prv.y1,nxt.y1), avg(prv.g0,nxt.g0)

    auto tmp0 = vrhaddq_u8( nxt_p0, prv_p0 );                     // epu8[] = ..., avg(prv.y3,nxt.y3), avg(prv.g2,nxt.g2), avg(prv.y1,nxt.y1), avg(prv.g0,nxt.g0)
    auto tmp2 = vreinterpret_u8( vshrq_n_u16( vreinterpret_u16( tmp0 ), 8 ) );

    return vbslq_u8( mask_0xFF00, tmp1, tmp2 );
}

FORCEINLINE uint8x16_t calc_y_from_gx_line( uint8x16_t prv_p0, uint8x16_t prv_p2, uint8x16_t nxt_p0, uint8x16_t nxt_p2 )
{
    // prv = y3g3y2g1y0
    auto prv_avg_line = vrhaddq_u8( prv_p0, prv_p2 );             // epu8[] =  ..., avg(g3, g5), avg(y2,y4), avg(g1,g3), avg(y2,y0)
    auto nxt_avg_line = vrhaddq_u8( nxt_p0, nxt_p2 );
    auto tmp1 = vrhaddq_u8( prv_avg_line, nxt_avg_line );         // 

    auto tmp0 = vrhaddq_u8( nxt_p2, prv_p2 );                     // epu8[] = ..., __, avg(prv.y2,nxt.y2), __, avg(prv.y0,nxt.y0)
    auto tmp2 = vreinterpret_u8( vshlq_n_u16( vreinterpret_u16( tmp0 ), 8 ) );

    return vbslq_u8( mask_0x00FF, tmp1, tmp2 );
}

FORCEINLINE uint16x8_t calc_edge_g( uint16x8_t cur_g_p0, uint16x8_t cur_g_p2, uint16x8_t prv_g, uint16x8_t nxt_g )
{
    auto sum_lr = vrhaddq_u16( cur_g_p0, cur_g_p2 );
    auto sum_ab = vrhaddq_u16( prv_g, nxt_g );
    auto sum_al = vrhaddq_u16( sum_lr, sum_ab );           // sum(prv[0],nxt[0],cur[-1],cur[+1]) / 4

    auto dif_lr = vabdq_u16( cur_g_p0, cur_g_p2 );
    auto dif_ab = vabdq_u16( prv_g, nxt_g );

    auto cmp_lt = vcltq_u16( dif_lr, dif_ab );        // epi16[] = ..., (dif_lr[x] <  dif_ab[x] ? 0xFFFF : 0x0000)
    auto cmp_eq = vceqq_u16( dif_lr, dif_ab );        // epi16[] = ..., (dif_lr[x] == dif_ab[x] ? 0xFFFF : 0x0000)

    auto tmp0 = vbslq_u16( cmp_lt, sum_lr, sum_ab );  // epi16[] = ..., (dif_lr[x] <  dif_ab[x] ? sum_ab : sum_lr)
    auto tmp1 = vbslq_u16( cmp_eq, sum_al, tmp0 );
    
    return tmp1;
}

FORCEINLINE uint8x16_t calc_edge_g_v2( uint8x16_t cur_g_p0, uint8x16_t cur_g_p2, uint8x16_t prv_g, uint8x16_t nxt_g )
{
    auto sum_lr = vrhaddq_u8( cur_g_p0, cur_g_p2 );
    auto sum_ab = vrhaddq_u8( prv_g, nxt_g );
    auto sum_al = vrhaddq_u8( sum_lr, sum_ab );           // sum(prv[0],nxt[0],cur[-1],cur[+1]) / 4

    auto dif_lr = vabdq_u8( cur_g_p0, cur_g_p2 );
    auto dif_ab = vabdq_u8( prv_g, nxt_g );

    auto cmp_lt = vcltq_u8( dif_lr, dif_ab );        // epi16[] = ..., (dif_lr[x] <  dif_ab[x] ? 0xFFFF : 0x0000)
    auto cmp_eq = vceqq_u8( dif_lr, dif_ab );        // epi16[] = ..., (dif_lr[x] == dif_ab[x] ? 0xFFFF : 0x0000)

    auto tmp0 = vbslq_u8( cmp_lt, sum_lr, sum_ab );  // epi16[] = ..., (dif_lr[x] <  dif_ab[x] ? sum_ab : sum_lr)
    auto tmp1 = vbslq_u8( cmp_eq, sum_al, tmp0 );

    return tmp1;
}

FORCEINLINE uint8x16_t calc_g_from_xg_line( uint8x16_t prv_p2, uint8x16_t cur_p0, uint8x16_t cur_p2, uint8x16_t nxt_p2 )
{
    // prv/nxt epu16[] = ??gx
    // cur     epu16[] = gx??
    auto cur_g_p0 = vshrq_n_u16( vreinterpret_u16( cur_p0 ), 8 );     // epu16[x] = 00gx
    auto cur_g_p2 = vshrq_n_u16( vreinterpret_u16( cur_p2 ), 8 );

    auto tmp1_ = calc_edge_g_v2( vreinterpret_u8( cur_g_p0 ), vreinterpret_u8( cur_g_p2 ), prv_p2, nxt_p2 );
    auto tmp3 = vshlq_n_u16( vreinterpret_u16( tmp1_ ), 8 );

    return vreinterpret_u8( vorrq_u16( cur_g_p0, tmp3 ) );
}

FORCEINLINE uint8x16_t calc_g_from_gx_line( uint8x16_t prv, uint8x16_t cur_p0, uint8x16_t cur_p2, uint8x16_t nxt )
{
    auto prv_g = vshrq_n_u16( vreinterpret_u16( prv ), 8 );
    auto nxt_g = vshrq_n_u16( vreinterpret_u16( nxt ), 8 );

    auto tmp2 = vshlq_n_u16( vreinterpret_u16( cur_p2 ), 8 );

    auto tmp1_ = calc_edge_g_v2( cur_p0, cur_p2, vreinterpret_u8( prv_g ), vreinterpret_u8( nxt_g ) );
    auto tmp1 = vreinterpret_u16( vandq_u8( tmp1_, mask_0x00FF ) );

    return vreinterpret_u8( vorrq_u16( tmp1, tmp2 ) );
}

FORCEINLINE uint8x16_t calc_avgG_value( uint16x8_t prv_g_p0, uint16x8_t prv_g_p2, uint16x8_t nxt_g_p0, uint16x8_t nxt_g_p2, uint16x8_t cur_g )
{
    const uint16x8_t mask_0x0007 = INIT_Neon_u16x8( 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007, 0x0007 );

    auto dif_lr = vabdq_u16( prv_g_p0, prv_g_p2 );
    auto dif_ab = vabdq_u16( prv_g_p0, nxt_g_p0 );

    auto t0 = vrhaddq_u16( prv_g_p0, prv_g_p2 );
    auto t1 = vrhaddq_u16( nxt_g_p0, nxt_g_p2 );

    auto sum_all = vrhaddq_u16( t0, t1 );
    sum_all = vrhaddq_u16( sum_all, cur_g );


    /*
        int dH = abs( prv[-1] - prv[+1] );
        int dV = abs( prv[-1] - nxt[-1] );
        if( 0x07 > dH && 0x07 > dV ) {
            return (prv[-1] + prv[1] + nxt[-1] + nxt[1] + cur[0] * 4) / 8
        } else {
            return cur[0];
        }
    */

    auto diff_gt_7 = vcltq_u16( dif_ab, mask_0x0007 );
    auto diff_lr_7 = vcltq_u16( dif_lr, mask_0x0007 );

    auto cond_true = vandq_u16( diff_gt_7, diff_lr_7 );

    return vreinterpret_u8( vbslq_u16( cond_true, sum_all, cur_g ) );
}

FORCEINLINE uint8x16_t calc_g_from_xg_line_avgG( uint8x16_t prv_p0, uint8x16_t prv_p2, uint8x16_t cur_p0, uint8x16_t cur_p2, uint8x16_t nxt_p0, uint8x16_t nxt_p2 )
{
    // prv/nxt epu16[] = ??gx
    // cur     epu16[] = gx??

    auto cur_g_p0 = vshrq_n_u16( vreinterpret_u16( cur_p0 ), 8 );     // epu16[x] = 00gx
    auto cur_g_p2 = vshrq_n_u16( vreinterpret_u16( cur_p2 ), 8 );

    auto prv_g_p0 = vreinterpret_u16( vandq_u8( prv_p0, mask_0x00FF ) );
    auto nxt_g_p0 = vreinterpret_u16( vandq_u8( nxt_p0, mask_0x00FF ) );

    auto prv_g_p2 = vreinterpret_u16( vandq_u8( prv_p2, mask_0x00FF ) );
    auto nxt_g_p2 = vreinterpret_u16( vandq_u8( nxt_p2, mask_0x00FF ) );

    auto tmp1 = calc_edge_g( cur_g_p0, cur_g_p2, prv_g_p2, nxt_g_p2 );
    auto tmp2 = vshlq_n_u16( tmp1, 8 );

    auto g_even = calc_avgG_value( prv_g_p0, prv_g_p2, nxt_g_p0, nxt_g_p2, cur_g_p0 );

    auto res = vorrq_u8( g_even, vreinterpret_u8( tmp2 ) );
    return res;
}

FORCEINLINE uint8x16_t calc_g_from_gx_line_avgG( uint8x16_t prv_p0, uint8x16_t prv_p2, uint8x16_t cur_p0, uint8x16_t cur_p2, uint8x16_t nxt_p0, uint8x16_t nxt_p2 )
{
    // prv/nxt epu16[] = ??gx
    // cur     epu16[] = gx??

    auto cur_g_p0 = vreinterpret_u16( vandq_u8( cur_p0, mask_0x00FF ) );     // epu16[x] = 00gx
    auto cur_g_p2 = vreinterpret_u16( vandq_u8( cur_p2, mask_0x00FF ) );

    auto prv_g_p0 = vshrq_n_u16( vreinterpret_u16( prv_p0 ), 8 );
    auto nxt_g_p0 = vshrq_n_u16( vreinterpret_u16( nxt_p0 ), 8 );

    auto prv_g_p2 = vshrq_n_u16( vreinterpret_u16( prv_p2 ), 8 );
    auto nxt_g_p2 = vshrq_n_u16( vreinterpret_u16( nxt_p2 ), 8 );

    auto tmp1 = calc_edge_g( cur_g_p0, cur_g_p2, prv_g_p0, nxt_g_p0 );

    auto g_even = calc_avgG_value( prv_g_p0, prv_g_p2, nxt_g_p0, nxt_g_p2, cur_g_p2 );
    auto tmp2 = vshlq_n_u16( vreinterpret_u16( g_even ), 8 );

    auto res = vorrq_u16( tmp1, tmp2 );
    return vreinterpret_u8( res );
}

template<bool use_avg_green,by_pattern pat>
FORCEINLINE void    conv_sse_reg( uint8x16_t& r, uint8x16_t& g, uint8x16_t& b,
    uint8x16_t prv_p0, uint8x16_t cur_p0, uint8x16_t nxt_p0,
    uint8x16_t prv_p2, uint8x16_t cur_p2, uint8x16_t nxt_p2 )
{
    uint8x16_t x_chn, y_chn, g_chn;
    if constexpr( is_gx_line( pat ) )
    {
        x_chn = calc_x_from_gx_line( cur_p0, cur_p2 );
        y_chn = calc_y_from_gx_line( prv_p0, prv_p2, nxt_p0, nxt_p2 );

        if constexpr( use_avg_green ) {
            g_chn = calc_g_from_gx_line_avgG( prv_p0, prv_p2, cur_p0, cur_p2, nxt_p0, nxt_p2 );
        } else {
            g_chn = calc_g_from_gx_line( prv_p0, cur_p0, cur_p2, nxt_p0 );
        }
    } else {
        x_chn = calc_x_from_xg_line( cur_p0, cur_p2 );
        y_chn = calc_y_from_xg_line( prv_p0, prv_p2, nxt_p0, nxt_p2 );

        if constexpr( use_avg_green ) {
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


template<class TOut,by_pattern pat, bool use_mtx, bool use_avg_green>
void	    conv_line( const alg_context& clr, const line_data& lines_, int dim_x )
{
    auto lines = lines_;
    
    constexpr auto nxt_pattern = next_pixel( pat );
    
    {
        auto prv_lo_ = vld1q_u8( lines.lines[0] + 0 );
        auto cur_lo_ = vld1q_u8( lines.lines[1] + 0 );
        auto nxt_lo_ = vld1q_u8( lines.lines[2] + 0 );

        static const uint8x16_t shuf_first_byte = INIT_Neon_u8x16( 1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 );
        auto prv_lo = vqtbl1q_u8( prv_lo_, shuf_first_byte );
        auto cur_lo = vqtbl1q_u8( cur_lo_, shuf_first_byte );
        auto nxt_lo = vqtbl1q_u8( nxt_lo_, shuf_first_byte );

        int x = 0;

        for( ; x < (dim_x - 32); x += 16 )
        {
            // reads [x + 16;x + 32[
            auto prv_hi = vld1q_u8( lines.lines[0] + x + 16 - 1 );
            auto cur_hi = vld1q_u8( lines.lines[1] + x + 16 - 1 );
            auto nxt_hi = vld1q_u8( lines.lines[2] + x + 16 - 1 );

            auto prv_p2 = vextq_u8( prv_lo, prv_hi, 2 );
            auto cur_p2 = vextq_u8( cur_lo, cur_hi, 2 );
            auto nxt_p2 = vextq_u8( nxt_lo, nxt_hi, 2 );

            uint8x16_t r, g, b;
            conv_sse_reg<use_avg_green, nxt_pattern>( r, g, b, prv_lo, cur_lo, nxt_lo, prv_p2, cur_p2, nxt_p2 );

            if constexpr( use_mtx ) {
                apply_color_matrix( clr, r, g, b );
            }

            store<TOut>( lines, x, r, g, b );       // writes [x + 1; x + 1 + 16[

            prv_lo = prv_hi;
            cur_lo = cur_hi;
            nxt_lo = nxt_hi;
        }

        if( x < (dim_x - 16 - 2 + 1) )      // x  = dim_x - 32
        {
            auto prv_p2 = vld1q_u8( lines.lines[0] + x + 2 - 1 );
            auto cur_p2 = vld1q_u8( lines.lines[1] + x + 2 - 1 );
            auto nxt_p2 = vld1q_u8( lines.lines[2] + x + 2 - 1 );

            uint8x16_t r, g, b;
            conv_sse_reg<use_avg_green, nxt_pattern>( r, g, b, prv_lo, cur_lo, nxt_lo, prv_p2, cur_p2, nxt_p2 );

            if constexpr( use_mtx ) {
                apply_color_matrix( clr, r, g, b );
            }

            store<TOut>( lines, x, r, g, b );       // writes [x + 1; x + 1 + 16[
        }
    }

    if( dim_x >= 18 )
    {
        // writes pixel [dim_x - 16;dim_x[
        // reads pixel  [dim_x - 32;dim_x[
        auto prv_minus_step = vld1q_u8( lines.lines[0] + dim_x - 16 );
        auto cur_minus_step = vld1q_u8( lines.lines[1] + dim_x - 16 );
        auto nxt_minus_step = vld1q_u8( lines.lines[2] + dim_x - 16 );

        auto prv_p0 = vld1q_u8( lines.lines[0] + dim_x - 16 - 2 );
        auto cur_p0 = vld1q_u8( lines.lines[1] + dim_x - 16 - 2 );
        auto nxt_p0 = vld1q_u8( lines.lines[2] + dim_x - 16 - 2 );

        uint8x16_t r, g, b;
        conv_sse_reg<use_avg_green, pat>( r, g, b, prv_p0, cur_p0, nxt_p0, prv_minus_step, cur_minus_step, nxt_minus_step );

        if constexpr( use_mtx ) {
            apply_color_matrix( clr, r, g, b );
        }

        // double last entry, to get the equivalent to pixel copy
        static const uint8x16_t shuf_last_byte = INIT_Neon_u8x16( 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 15 );
        r = vqtbl1q_u8( r, shuf_last_byte );
        g = vqtbl1q_u8( g, shuf_last_byte );
        b = vqtbl1q_u8( b, shuf_last_byte );

        int write_index = dim_x - 16;

        store<TOut>( lines, write_index, r, g, b );
    }
}

template<class TOut,bool ... TboolParams>
void	convert_by8_to_rgb_edge_neonv7( by_pattern pattern, const line_data& lines, int dim_x, const alg_context& ctx )
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
    if( ctx.use_color_matrix ) {
        if( ctx.use_avg_green ) {
            convert_by8_to_rgb_edge_neonv7<TOutDataType, true,true>( pattern, lines, dim_x, ctx );
        } else  {
            convert_by8_to_rgb_edge_neonv7<TOutDataType, true,false>( pattern, lines, dim_x, ctx );
        }
    } else {
        if( ctx.use_avg_green ) {
            convert_by8_to_rgb_edge_neonv7<TOutDataType, false,true>( pattern, lines, dim_x, ctx );
        } else  {
            convert_by8_to_rgb_edge_neonv7<TOutDataType, false,false>( pattern, lines, dim_x, ctx );
        }
    }
}



alg_context_sse     fill_context( const img_filter::transform::by_edge::options& in_opt )
{
    auto ctx = alg_context_sse{ {}, in_opt.use_color_matrix, in_opt.use_avg_green };
    for( int i = 0; i < 9; ++i )
    {
        ctx.clr_mtx[i] = vdupq_n_s16( in_opt.color_mtx.fac[i] );
    }

    return ctx;
}

template<typename TRGBStr>
static void by_edge_image_loop_neon( img::img_descriptor dst_, img::img_descriptor src, const img_filter::transform::by_edge::options& options )
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


img_filter::transform::by_edge::function_type   img_filter::transform::by_edge::get_transform_by8_to_dst_neon( img::img_type dst, img::img_type src )
{
    if( !img::is_by8_fcc( src.fourcc_type() ) || dst.dim != src.dim ) {
        return nullptr;
    }
    if( dst.dim.cx < 32 || dst.dim.cy < 2 ) {
        return nullptr;
    }

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGRA32: return &by_edge_image_loop_neon<BGRA32>;
    case img::fourcc::BGR24: return &by_edge_image_loop_neon<BGR24>;
    default:
        return nullptr;
    };
}
