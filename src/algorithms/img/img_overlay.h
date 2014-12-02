
#ifndef IMG_OVERLAY_H_INC_
#define IMG_OVERLAY_H_INC_

#pragma once

#include "../image_transform_base.h"

namespace img
{
namespace overlay
{
	typedef enum {

		BLACK = 0,
		WHITE,
		YELLOW,
		CYAN,
		GREEN,
		MAGENTA,
		RED,
		BLUE,
		GREY,

		MAX_COLOR,
		TRANSPARENT_CLR,

	} COLOR;

#define POSITION_CENTER ((int)-1)

	void	RenderText( img_descriptor& data, POINT pos, unsigned int scaling, const char* pText, 
		COLOR BgColor, COLOR FgColor, bool bImgIsFlippedVertical = false );
};
};

#endif // IMG_OVERLAY_H_INC_
