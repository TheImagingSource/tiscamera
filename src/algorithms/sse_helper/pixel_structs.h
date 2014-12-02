
#pragma once

#include "../image_transform_interop.h"

namespace img_transform
{
	struct UYVY
	{
		byte	u, y0, v, y1;
	};

	struct YUY2
	{
		byte	y0, u, y1, v;
	};

	struct IYU2
	{
		byte	u0, y0, v0;
	};

	struct AYUV
	{
		byte	u0, y0, v0, a0;
	};

	struct RGB32
	{
		byte b, g, r, a;
	};

    struct RGB24
    {
        byte b, g, r;
    };

	typedef AYUV YUV32;

    struct YUV48
    {
        uint16_t y0, u0, v0;
    };

    struct RGB64
    {
        uint16_t b, g, r, a;
    };
};