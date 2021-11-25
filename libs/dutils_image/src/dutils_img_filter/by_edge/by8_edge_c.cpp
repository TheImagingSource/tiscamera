

#include "by_edge.h"

#include "by_edge_internal.h"

#include "by8_pixelops.h"

namespace
{
    using alg_context = by_edge_internal::alg_context_c;

    struct pixel
    {
        uint8_t r, g, b;
    };

using namespace by_edge_internal;
using namespace img::by_transform::by_pattern_alg;

template<class TOutStruct>
void    store( void* p_dest, int x, pixel val ) = delete;

template<>
FORCEINLINE void    store<BGRA32>( void* p_dest, int x, pixel val )
{
    BGRA32* out_line = reinterpret_cast<BGRA32*>(p_dest);
    out_line[x] = BGRA32{ val.b, val.g, val.r, 0xFF };
}

template<>
FORCEINLINE void    store<BGR24>( void* p_dest, int x, pixel val )
{
    BGR24* out_line = reinterpret_cast<BGR24*>(p_dest);
    out_line[x] = BGR24{ val.b, val.g, val.r };
}

FORCEINLINE  pixel	apply_color_matrix_c( const img::color_matrix_int& clr, pixel str )
{
    int r = ((int) str.r * clr.r_rfac + (int) str.g * clr.r_gfac + (int) str.b * clr.r_bfac) / 64;
    int g = ((int) str.r * clr.g_rfac + (int) str.g * clr.g_gfac + (int) str.b * clr.g_bfac) / 64;
    int b = ((int) str.r * clr.b_rfac + (int) str.g * clr.b_gfac + (int) str.b * clr.b_bfac) / 64;

    r = CLIP( r, 0, 0xFF );
    g = CLIP( g, 0, 0xFF );
    b = CLIP( b, 0, 0xFF );

    return pixel{ (uint8_t)r, (uint8_t) g, (uint8_t)b };
}

template<by_pattern pattern,bool TAvgG>
FORCEINLINE pixel		conv_edge( int idxInLine, const line_data& lines )
{
    pixel out;

    uint8_t* prv_line = lines.lines[0] + idxInLine;
    uint8_t* cur_line = lines.lines[1] + idxInLine;
    uint8_t* nxt_line = lines.lines[2] + idxInLine;

    if constexpr( pattern == by_pattern::GR ) {	// red line
        if constexpr( TAvgG )  {
            CALC_RGLINE_G_avg(prv_line,cur_line,nxt_line,out);
        } else {
            CALC_RGLINE_G(prv_line,cur_line,nxt_line,out);
        }
    }
    else if constexpr( pattern == by_pattern::RG )
    {
        CALC_RGLINE_R_edge(prv_line,cur_line,nxt_line,out);
    }
    else if constexpr( pattern == by_pattern::GB )
    { // blue line
        if constexpr( TAvgG )  {
            CALC_GBLINE_G_avg(prv_line,cur_line,nxt_line,out);
        } else {
            CALC_GBLINE_G(prv_line,cur_line,nxt_line,out);
        }
    }
    else // if( pattern == by_pattern::BG )
    {
        CALC_GBLINE_B_edge(prv_line,cur_line,nxt_line,out);
    }
    return out;
}

template<by_pattern pattern, bool apply_clr, bool avg_green>
FORCEINLINE pixel		conv_edge_with_clr( const alg_context& clr, int idxInLine, const line_data& lines )
{
    pixel out = conv_edge<pattern,avg_green>( idxInLine, lines );
    if constexpr( apply_clr ) {
        return apply_color_matrix_c( clr.color_mtx, out );
    }
    else {
        return out;
    }
}

template<class TOut,by_pattern pattern, bool use_mtx, bool use_avg_green>
int	conv_line_c( const alg_context& clr, const line_data& lines, int x, int dim_x )
{
    constexpr auto cur_pattern = pattern;
    constexpr auto nxt_pattern = next_pixel( pattern );

    for( /*x = 2*/; x < (dim_x - 2); x += 2 )
    {
        auto tmp0 = conv_edge_with_clr<cur_pattern, use_mtx, use_avg_green>( clr, x + 0, lines );
        store<TOut>( lines.out_line, x + 0, tmp0 );
        auto tmp1 = conv_edge_with_clr<nxt_pattern, use_mtx, use_avg_green>( clr, x + 1, lines );
        store<TOut>( lines.out_line, x + 1, tmp1 );
    }
    return x;
}

template<by_pattern pattern,class TOut, bool use_mtx,bool use_avg_green = true>
void	convert_by8_to_rgb_edge( const alg_context& clr, const line_data& lines, int dim_x )
{
    constexpr auto cur_pattern = pattern;
    constexpr auto nxt_pattern = next_pixel( pattern );

    pixel tmp = conv_edge_with_clr<nxt_pattern, use_mtx, use_avg_green>( clr, 0 + 1, lines );
    store<TOut>( lines.out_line, 0 + 0, tmp );
    store<TOut>( lines.out_line, 0 + 1, tmp );

    int x = conv_line_c<TOut,cur_pattern,use_mtx,use_avg_green>( clr, lines, 2, dim_x );

    // x = dim_cx - 2
    tmp = conv_edge_with_clr<pattern, use_mtx, use_avg_green>( clr, x + 0, lines );;

    store<TOut>( lines.out_line, x + 0, tmp );
    store<TOut>( lines.out_line, x + 1, tmp );
}

template<class TOutDataType,bool use_mtx,bool avg_greenx>
static void transform_line_( by_pattern pattern, const line_data& lines, int dim_x, const alg_context& ctx )
{
    switch( pattern )
    {
    case by_pattern::BG:    convert_by8_to_rgb_edge<by_pattern::BG, TOutDataType,use_mtx, avg_greenx>( ctx, lines, dim_x );    break;
    case by_pattern::GB:    convert_by8_to_rgb_edge<by_pattern::GB, TOutDataType,use_mtx, avg_greenx>( ctx, lines, dim_x );    break;
    case by_pattern::GR:    convert_by8_to_rgb_edge<by_pattern::GR, TOutDataType,use_mtx, avg_greenx>( ctx, lines, dim_x );    break;
    case by_pattern::RG:    convert_by8_to_rgb_edge<by_pattern::RG, TOutDataType,use_mtx, avg_greenx>( ctx, lines, dim_x );    break;
    };
}

template<class TOutDataType>
static void transform_line( by_pattern pattern, const line_data& lines, int dim_x, const alg_context& ctx )
{
    if( ctx.use_color_matrix ) {
        if( ctx.use_avg_green ) {
            transform_line_<TOutDataType, true,true>( pattern, lines, dim_x, ctx );
        } else  {
            transform_line_<TOutDataType, true,false>( pattern, lines, dim_x, ctx );
        }
    } else {
        if( ctx.use_avg_green ) {
            transform_line_<TOutDataType, false,true>( pattern, lines, dim_x, ctx );
        } else  {
            transform_line_<TOutDataType, false,false>( pattern, lines, dim_x, ctx );
        }
    }
}


template<typename TRGBStr>
static void by_edge_image_loop( img::img_descriptor dst_, img::img_descriptor src, const img_filter::transform::by_edge::options& in_opt )
{
    const auto dst = flip_image_in_img_desc_if_allowed( dst_ );

    const by_pattern pattern_cur = img::by_transform::convert_bayer_fcc_to_pattern( src.fourcc_type() );
    const by_pattern pattern_nxt = img::by_transform::by_pattern_alg::next_line( pattern_cur );

    const int dim_y = src.dim.cy;

    if( !(src.flags & img::img_descriptor::flags_no_wrap_beg) ) {
        transform_line<TRGBStr>( pattern_cur, init_src_param( 0, dst, src, +1, +1 ), src.dim.cx, in_opt );
    }
    else {
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
    }
    else {
        transform_line<TRGBStr>( pattern_nxt, init_src_param( y, dst, src, -1, +1 ), src.dim.cx, in_opt );
    }
}

}

template<class TOutDataType, bool b0, bool b1>
static int  map_pattern( by_pattern pattern, const alg_context& ctx, const line_data& lines, int x, int dim_x )
{
    switch( pattern )
    {
    case by_pattern::BG:    return ::conv_line_c<TOutDataType, by_pattern::BG, b0, b1>( ctx, lines, x, dim_x );    break;
    case by_pattern::GB:    return ::conv_line_c<TOutDataType, by_pattern::GB, b0, b1>( ctx, lines, x, dim_x );    break;
    case by_pattern::GR:    return ::conv_line_c<TOutDataType, by_pattern::GR, b0, b1>( ctx, lines, x, dim_x );    break;
    case by_pattern::RG:    return ::conv_line_c<TOutDataType, by_pattern::RG, b0, b1>( ctx, lines, x, dim_x );    break;
    };
    return 0;
}

template<>
int	    by_edge_internal::conv_by8_line_c<BGRA32>( by_pattern pattern, const alg_context_c& clr, const line_data& lines, int x, int dim_x )
{
    if( clr.use_color_matrix ) {
        if( clr.use_avg_green ) {
            return map_pattern<BGRA32, true, true>( pattern, clr, lines, x, dim_x );
        }
        else {
            return map_pattern<BGRA32, true, false>( pattern, clr, lines, x, dim_x );
        }
    }
    else {
        if( clr.use_avg_green ) {
            return map_pattern<BGRA32, false, true>( pattern, clr, lines, x, dim_x );
        }
        else {
            return map_pattern<BGRA32, false, false>( pattern, clr, lines, x, dim_x );
        }
    }
}

template<>
int	    by_edge_internal::conv_by8_line_c<BGR24>( by_pattern pattern, const alg_context_c& clr, const line_data& lines, int x, int dim_x )
{
    if( clr.use_color_matrix ) {
        if( clr.use_avg_green ) {
            return map_pattern<BGR24, true, true>( pattern, clr, lines, x, dim_x );
        }
        else {
            return map_pattern<BGR24, true, false>( pattern, clr, lines, x, dim_x );
        }
    }
    else {
        if( clr.use_avg_green ) {
            return map_pattern<BGR24, false, true>( pattern, clr, lines, x, dim_x );
        }
        else {
            return map_pattern<BGR24, false, false>( pattern, clr, lines, x, dim_x );
        }
    }
}

img_filter::transform::by_edge::function_type   img_filter::transform::by_edge::get_transform_by8_to_dst_c( img::img_type dst, img::img_type src )
{
    if( !img::is_by8_fcc( src.fourcc_type() ) || dst.dim != src.dim ) {
        return nullptr;
    }
    if( dst.dim.cx < 4 || dst.dim.cy < 2 ) {
        return nullptr;
    }

    switch( dst.fourcc_type() )
    {
    case img::fourcc::BGRA32: return &by_edge_image_loop<BGRA32>;
    case img::fourcc::BGR24: return &by_edge_image_loop<BGR24>;
    default:
        return nullptr;
    };
}
