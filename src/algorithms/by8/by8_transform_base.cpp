

#include "by8_transform_base.h"

namespace by8_transform
{

inline byte* calc_output_offset( bool TbFlip, unsigned int dim_cy, unsigned int pitch,
								byte* pDataStart, unsigned int y )
{
	if( TbFlip ) {
		return pDataStart + (dim_cy - y - 1) * pitch;
	}
	else
	{
		return pDataStart + y * pitch;
	}
}

FORCEINLINE unsigned int	 calc_pattern_for_line( unsigned int start_pattern, unsigned int idx )
{
	static const unsigned int arr[4] = {
		/*BG = 0, => */ GR,
		/*GB, => */ RG,
		/*GR, => */ BG,
		/*RG, => */ GB,
	};
	if( idx % 2 == 0 )
		return start_pattern;
	return arr[start_pattern];
}

void	call_func(  alg_context_params& alg_context, convert_line_function pFunc, const line_data& line, unsigned int pattern )
{
	pFunc( alg_context, line, pattern );
	//pFunc( pattern == BG || pattern == GB, pattern == GR || pattern == GB, pContext, line );
}

void	by8_convert_func( img::img_descriptor& dest, const by8_add_packet& pckt, unsigned int pattern, bool bFlipH, const alg_context_params& alg_context, convert_line_function pFunc )
{
	// pointers must be valid
	ASSERT( dest.pData && pckt.pLines );
	ASSERT( dest.dim_y > 0 && dest.pitch && dest.dim_x >= 4 );
	// we don't know the proportion of pitch to dim, so just check for greater-equal

	ASSERT( pckt.line_count > 2 );	// smaller then 2 is a bit superfluous, but should work ...
	ASSERT( pckt.start_y < dest.dim_y );	// the start index must be inside the destination rect
	ASSERT( pckt.pitch >= dest.dim_x );		// we need at least pixel_per_line bytes in the source to fill the dest

	bFlipH = !bFlipH;

	unsigned int src_pitch = pckt.pitch;
	unsigned int y = pckt.start_y;

	int temp_dest_pitch;
	if( bFlipH )
		temp_dest_pitch = -int(dest.pitch);
	else
		temp_dest_pitch = dest.pitch;

	line_data lines = {
		pckt.pLines + src_pitch * (-1),	// prev
		pckt.pLines + src_pitch * (0),		// cur
		pckt.pLines + src_pitch * (1),	// next
		calc_output_offset( bFlipH, dest.dim_y, dest.pitch, dest.pData, y ),
		dest.dim_x
	};

	if( pckt.pPreFirstLine )
	{	// first line
		lines.pPrevLine = pckt.pPreFirstLine;

		pFunc( alg_context, lines, calc_pattern_for_line( pattern, y ) );

		lines.pPrevLine = lines.pCurLine;
		lines.pCurLine += src_pitch;
		lines.pNextLine += src_pitch;

		lines.pOutLine += temp_dest_pitch;
		++y;
	}
	{
		unsigned int lines_to_convert = pckt.line_count - (pckt.pPostLastLine ? 1 : 0);
		unsigned int steps = lines_to_convert / 2;
		if( steps )
		{
			switch( y % 2 )
			{
				do
				{
			case 0:
					pFunc( alg_context, lines, calc_pattern_for_line( pattern, 0 ) );

					lines.pPrevLine += src_pitch;
					lines.pCurLine += src_pitch;
					lines.pNextLine += src_pitch;

					lines.pOutLine += temp_dest_pitch;

					++y;
			case 1:
					pFunc( alg_context, lines, calc_pattern_for_line( pattern, 1 ) );

					lines.pPrevLine += src_pitch;
					lines.pCurLine += src_pitch;
					lines.pNextLine += src_pitch;

					lines.pOutLine += temp_dest_pitch;

					++y;
				}
				while( --steps );
			};
		}
		if( lines_to_convert % 2 )
		{
			pFunc( alg_context, lines, calc_pattern_for_line( pattern, y ) );

			lines.pPrevLine += src_pitch;
			lines.pCurLine += src_pitch;
			lines.pNextLine += src_pitch;

			lines.pOutLine += temp_dest_pitch;
			y += 1;
		}

		if( pckt.pPostLastLine )
		{
			lines.pNextLine = pckt.pPostLastLine;
			pFunc( alg_context, lines, calc_pattern_for_line( pattern, y ) );
		}
	}
}

};
