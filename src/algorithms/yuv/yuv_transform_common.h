
#pragma once

#include "../sse_helper/sse_utils.h"
#include "../sse_helper/pixel_structs.h"

namespace {
    using namespace img_transform;

template<class TDest,class TSrc>
inline void	conv_YUV888_to_RGB888_int( TDest& dest, const TSrc& src )
{
	static const int fac = 64;

	int u = src.u0;
	int v = src.v0	;
	int y0 = int(src.y0) * fac;
	// M2 code from Intel developer site
	int r0 = y0 + 87 * (v - 128);		// * 87,72512
	int g0 = y0 - (45 * (v - 128))		// * 44,672064
		- (21 * (u - 128));				// * 21,608512
	int b0 = y0 + 111 * (u - 128);		// * 110,876544

	r0 /= fac; g0 /= fac; b0 /= fac;

	dest.r = (byte) CLIP( r0, 0, 0xFF );
	dest.g = (byte) CLIP( g0, 0, 0xFF );
	dest.b = (byte) CLIP( b0, 0, 0xFF );
	//dest.a = 0;
}

template<class TDest,class TSrc>
inline void	conv_RGB888_to_YUV888_int( TDest& dest, const TSrc& src )
{
	int yt, ut, vt;

	yt = (int)(256 * 0.299f) * src.r + (int)(256 * 0.587f) * src.g + (int)(256 * 0.114f) * src.b;
	ut = (int)(256 * -0.173f) * src.r - (int)(256 * 0.339f) * src.g + (int)(256 * 0.512f) * src.b + 128 * 256;
	vt = (int)(256 * 0.512f) * src.r - (int)(256 * 0.427f) * src.g - (int)(256 * 0.084f) * src.b + 128 * 256;

	yt >>= 8;
	ut >>= 8;
	vt >>= 8;

	dest.y0 = (byte) CLIP( yt, 0, 0xFF );
	dest.u0 = (byte) CLIP( ut, 0, 0xFF );
	dest.v0 = (byte) CLIP( vt, 0, 0xFF );
}


};
