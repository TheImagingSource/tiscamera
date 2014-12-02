
#ifndef BY8TORGB_CONV_H_INC_
#define BY8TORGB_CONV_H_INC_

#pragma once

#include "../img/cpu_features.h"
#include "by8_base.h"

namespace by8_transform
{
	using namespace win32::optimization;


struct transform_by8_options 
{
	enum parameter_options
	{
		NoOptions = 0x0,
		UseClrMatrix = 0x1,
		UseAvgGreen = 0x2,
        FlipImage = 0x4,
	};
	unsigned int	options;
	// same as GetCPUFeatures @see (this could be a using, but to prevent problems with include files, we just replicate it here)
	//enum cpu_features
	//{
	//	CPU_NOFEATURES = 0x0,
	//	CPU_SSE2 = 0x4,
	//	CPU_SSE3 = 0x8,		
	//	CPU_SSSE3 = 0x10,	
	//	CPU_SSE41 = 0x20,	
	//	CPU_SSE42 = 0x40,	
	//	CPU_SSE4a = 0x80,	
	//};

	unsigned int	opt_level;
	color_matrix	color;
};

void	transform_by8_to_dest( img::img_descriptor& dest, const img::img_descriptor& src, const transform_by8_options& in_opt );

};

#endif // BY8TORGB_CONV_H_INC_