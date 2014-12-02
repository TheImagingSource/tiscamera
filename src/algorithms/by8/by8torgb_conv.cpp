
#include "by8torgb_conv.h"

#include "by8_transform_base.h"

using namespace by8_transform;

namespace by8_transform
{

convert_line_function	getfunc_by8_to_rgb_c_transform( unsigned int alg_opt, output_type type );

};

static convert_line_function init_alg_context( const img::img_descriptor& dest, const transform_by8_options& in_opt, alg_context_params& alg_context )
{
	alg_context.plane_offset = 0;
	alg_context.alg_options = 0;
	alg_context.alg_options |= in_opt.options & transform_by8_options::UseAvgGreen ? AO_AverageGreen : 0;
	alg_context.alg_options |= in_opt.options & transform_by8_options::UseClrMatrix ? AO_UseClrMatrix : 0;

    output_type output_format;
    switch( dest.type )
    {
    case FOURCC_RGB32:          output_format = AO_RGB32; break;
    case FOURCC_RGB24:          output_format = AO_RGB24; break;
    case FOURCC_YUV8PLANAR:
        output_format = AO_YUV8P;
        alg_context.plane_offset = dest.dim_x * dest.dim_y;
        break;
    default:
        return 0;
    }

    if( alg_context.alg_options & AO_UseClrMatrix )
    {
        const color_matrix& clr = in_opt.color;
        for( int i = 0; i < 8; ++i )
        {
            alg_context.r_rfac[i] = clr.r_rfac;
            alg_context.r_gfac[i] = clr.r_gfac;
            alg_context.r_bfac[i] = clr.r_bfac;
            alg_context.g_rfac[i] = clr.g_rfac;
            alg_context.g_gfac[i] = clr.g_gfac;
            alg_context.g_bfac[i] = clr.g_bfac;
            alg_context.b_rfac[i] = clr.b_rfac;
            alg_context.b_gfac[i] = clr.b_gfac;
            alg_context.b_bfac[i] = clr.b_bfac;
        }
	}

	// fall back to C
	return getfunc_by8_to_rgb_c_transform( alg_context.alg_options, output_format );
}

static void transform_by8_to_dest( img::img_descriptor& dest, byte* pInput, tBY8Pattern pattern, const transform_by8_options& in_opt )
{
	alg_context_params alg_context;
    memset( &alg_context, 0, sizeof( alg_context ) );
	convert_line_function pFunc = init_alg_context(dest,in_opt, alg_context);
	ASSERT( pFunc != 0 );
	if( pFunc == 0 )
		return;


    bool bFlipH = (in_opt.options & transform_by8_options::FlipImage) != 0;

    unsigned int w = dest.dim_x, h = dest.dim_y;

	if( dest.type == FOURCC_YUV8PLANAR )
	{
		bFlipH = !bFlipH;
	}

	by8_add_packet src_data = {
		pInput + w,
		pInput,
		pInput + w * (h - 2),
		0, h, w
	};
	by8_convert_func( dest, src_data, pattern, bFlipH, alg_context, pFunc );
}

void by8_transform::transform_by8_to_dest( img::img_descriptor& dest, const img::img_descriptor& src, const transform_by8_options& in_opt )
{
    ::transform_by8_to_dest( dest, src.pData, convertFCCToPattern( src.type ), in_opt );
}
