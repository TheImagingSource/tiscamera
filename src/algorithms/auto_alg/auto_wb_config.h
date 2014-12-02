
#pragma once

#include "auto_alg.h"

const int NEARGRAY_MIN_BRIGHTNESS	= 10;
const int NEARGRAY_MAX_BRIGHTNESS	= 253;
const float NEARGRAY_MAX_COLOR_DEVIATION	= 0.25f;
const float NEARGRAY_REQUIRED_AMOUNT		= 0.08f;

const int MAX_STEPS = 20;
const int WB_IDENTITY = 64;
const int WB_MAX = 255;
const int BREAK_DIFF = 2;

template<unsigned int max>
static int clip( int x )
{
	if( x > max )	return max;
	if( x < 0 )		return 0;
	return x;
}

