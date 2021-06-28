
Translation units/.cpp files must include one of the use_simd_*.h headers to specify the max level of sse 
	support this cpp file supports.
	
	e.g.
		a.cpp
			#include "stdafx.h"
			
			#include "<path>/simd_helper/use_simd_ssse3.h"
			
			// all other includes go here
			
SSE inline functions in headers must be introduced by one of the "include_*.h" headers.

	e.g.
		a.h
			#ifndef GUARD
			#define GUARD
			
			#include "<path>/sse_helper/include_ssse3.h"
			
			// now use the specific intrinsics included from the specified SSE level
			
