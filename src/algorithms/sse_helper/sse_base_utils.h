
//////////////////////////////////////////////////////////////////////////
//
// This is the base header, where we define sse(2) things so that gcc/MSVC/KERNEL_DRIIVER will build successfully side by side
//  We include image_base_defines for stdint.h etc.
//
//
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "../image_transform_interop.h"


#ifndef CLIP
#define CLIP(val,l,h) ( (val) < (l) ? (l) : (val) > (h) ? (h): (val) )
#endif // !CLIP

#pragma warning( push )
#pragma warning( disable : 4127 ) // conditional expression is constant

namespace sse_utils
{
	inline
	void	flip_image_params( byte*& ptr, int& pitch, int dim_y )
	{
			ptr = (ptr + pitch * (dim_y - 1));
			pitch = -pitch;
	}

};

#pragma warning(pop)
