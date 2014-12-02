

#include "by8_transform_base.h"

#include "by8_pixelops.h"
#include "../sse_helper/pixel_structs.h"
#include "../yuv/yuv_transform_common.h"

namespace
{
using namespace by8_transform;
using img_transform::RGB32;
using img_transform::RGB24;

struct store_rgb32
{
    static void store_output( void* p_dest, unsigned int dest_index, const RGB32& val, int )
    {
        RGB32* pOutputLineStart = static_cast<RGB32*>(p_dest);
        pOutputLineStart[dest_index] = val;
    }
};

struct store_rgb24
{
    static void store_output( void* p_dest, unsigned int dest_index, const RGB32& val, int )
    {
        RGB24* pOutputLineStart = static_cast<RGB24*>(p_dest);
        pOutputLineStart[dest_index].r = val.r;
        pOutputLineStart[dest_index].g = val.g;
        pOutputLineStart[dest_index].b = val.b;
    }
};

struct store_YUV8P
{
    static void store_output( void* p_dest, unsigned int dest_index, const RGB32& val, int plane_size )
    {
        byte* p_y_plane = static_cast<byte*>(p_dest) + dest_index;
        byte* p_u_plane = p_y_plane + plane_size;
        byte* p_v_plane = p_u_plane + plane_size;
        img_transform::AYUV tmp;
        conv_RGB888_to_YUV888_int( tmp, val );

        *p_y_plane = tmp.y0;
        *p_u_plane = tmp.u0;
        *p_v_plane = tmp.v0;
    }
};

static void	apply_color_matrix_c( const sse_color_matrix& clr, RGB32& str )
{
    int r = str.r;
	int g = str.g;
	int b = str.b;

    r = ((int) str.r * clr.r_rfac[0] + (int) str.g * clr.r_gfac[0] + (int) str.b * clr.r_bfac[0]) / 64;
	g = ((int) str.r * clr.g_rfac[0] + (int) str.g * clr.g_gfac[0] + (int) str.b * clr.g_bfac[0]) / 64;
	b = ((int) str.r * clr.b_rfac[0] + (int) str.g * clr.b_gfac[0] + (int) str.b * clr.b_bfac[0]) / 64;

	r = CLIP( r, 0, 0xFF );
	g = CLIP( g, 0, 0xFF );
	b = CLIP( b, 0, 0xFF );

	str.r = (byte) r;
	str.g = (byte) g;
	str.b = (byte) b;
}

template<tBY8Pattern pattern,bool TAvgG>
FORCEINLINE RGB32		conv_edge( unsigned int idxInLine, const line_data& lines )
{
    RGB32 out;

	byte* pInputPrevLine = lines.pPrevLine + idxInLine;
	byte* pInputCurLine = lines.pCurLine + idxInLine;
	byte* pInputNextLine = lines.pNextLine + idxInLine;

	if( pattern == by8_transform::GR ) {	// red line
		if( TAvgG )  {
			CALC_RGLINE_G_avg(pInputPrevLine,pInputCurLine,pInputNextLine,out);
		} else {
			CALC_RGLINE_G(pInputPrevLine,pInputCurLine,pInputNextLine,out);
		}
	}
	else if( pattern == by8_transform::RG )
	{
		CALC_RGLINE_R_edge(pInputPrevLine,pInputCurLine,pInputNextLine,out);
	}
	else if( pattern == by8_transform::GB )
	{ // blue line
		if( TAvgG )  {
			CALC_GBLINE_G_avg(pInputPrevLine,pInputCurLine,pInputNextLine,out);
		} else {
			CALC_GBLINE_G(pInputPrevLine,pInputCurLine,pInputNextLine,out);
		}
	}
	else if( pattern == by8_transform::BG )
	{
		CALC_GBLINE_B_edge(pInputPrevLine,pInputCurLine,pInputNextLine,out);
	}

    return out;
}


template<class options,tBY8Pattern pattern>
FORCEINLINE RGB32		conv_edge_with_clr( const alg_context_params& clr, unsigned int idxInLine, const line_data& lines )
{
    RGB32 out = conv_edge<pattern,options::AvgG>( idxInLine, lines );

    if( options::ApplyClrMtx )
        apply_color_matrix_c( clr, out );
    return out;
}

template<class options,tBY8Pattern pattern,class TOut>
static void	convert_by8_to_rgb_edge( const alg_context_params& clr, const line_data& lines )
{
    unsigned int x = 0;
    RGB32 tmp = conv_edge_with_clr<options,traits<pattern>::next_pixel>( clr, x + 1, lines );
    TOut::store_output( lines.pOutLine, 0, tmp, clr.plane_offset );
    TOut::store_output( lines.pOutLine, 1, tmp, clr.plane_offset );

    x = 2;
    for( /*x = 2*/; x < (lines.dim_cx - 2); x += 2 )
    {
        tmp = conv_edge_with_clr<options, pattern>( clr, x, lines );
        TOut::store_output( lines.pOutLine, x, tmp, clr.plane_offset );
        tmp = conv_edge_with_clr<options, traits<pattern>::next_pixel>( clr, x + 1, lines );
        TOut::store_output( lines.pOutLine, x + 1, tmp, clr.plane_offset );
    }

    // x = dim_cx - 2
    tmp = conv_edge_with_clr<options,pattern>( clr, x, lines );;

    TOut::store_output( lines.pOutLine, x, tmp, clr.plane_offset );
    TOut::store_output( lines.pOutLine, x + 1, tmp, clr.plane_offset );
}

template<unsigned int TParams,class TOutDataType>
static void convLine_edge( const alg_context_params& alg_context, const line_data& lines, unsigned int pattern )
{
    typedef by8_op_settings<TParams> options;
    switch( pattern ) {
    case BG:    convert_by8_to_rgb_edge<options,BG,TOutDataType>( alg_context, lines );    break;
    case GB:    convert_by8_to_rgb_edge<options,GB,TOutDataType>( alg_context, lines );    break;
    case GR:    convert_by8_to_rgb_edge<options,GR,TOutDataType>( alg_context, lines );    break;
    case RG:    convert_by8_to_rgb_edge<options,RG,TOutDataType>( alg_context, lines );    break;
    };
}

template<unsigned int TAlgParams,output_type type>
struct convLine_edge_c_
{
    static void 	convert_func_dispatcher( const alg_context_params& alg_context, const line_data& lines, unsigned int pattern )
	{
        if( type == AO_RGB32 )
            convLine_edge<TAlgParams, store_rgb32>( alg_context, lines, pattern );
		else if( type == AO_RGB24 )
			convLine_edge<TAlgParams,store_rgb24>( alg_context, lines, pattern );
        else if( type == AO_YUV8P )
            convLine_edge<TAlgParams, store_YUV8P>( alg_context, lines, pattern );
	}
	static convert_line_function	get()
	{
		return &convert_func_dispatcher;
	}
};
};

namespace by8_transform
{
    convert_line_function   getfunc_by8_to_rgb_c_transform( unsigned int alg_opt, output_type type )
    {
	    return find_expansion<convLine_edge_c_>( alg_opt, type );
    }
};
