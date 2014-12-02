
#ifndef BY8_PIXELOPS_H_INC_
#define BY8_PIXELOPS_H_INC_

#pragma once

#pragma warning ( disable : 4127 )

#include <math.h>

FORCEINLINE byte		edgesensing_aroundgreen( byte* _PPREVLINE, byte* _PCURLINE, byte* _PNEXTLINE )
{
	byte prev = _PCURLINE[-1];
	byte next = _PCURLINE[+1];
	byte above = _PPREVLINE[0];
	byte below = _PNEXTLINE[0];
	// input : green raster pos
	unsigned int dH = abs( prev - next );
	unsigned int dV = abs( above - below );
	
	if( dH < dV ) {
		return (prev + next) / 2;
	}
	else if( dH > dV ) {
		return (above + below ) / 2;
	}
	return (prev + next + above + below) / 4;
}



#define CALC_DIAGONAL(_PPREVLINE,_PCURLINE,_PNEXTLINE)		\
	((int) _PPREVLINE[-1] + (int) _PPREVLINE[+1] +			\
	(int) _PNEXTLINE[-1] + (int) _PNEXTLINE[+1]) / 4;

#define CALC_DIAGONAL_ongreen(_PPREVLINE,_PCURLINE,_PNEXTLINE)		\
	((int) _PPREVLINE[-1] + (int) _PPREVLINE[+1] +			\
	(int) _PNEXTLINE[-1] + (int) _PNEXTLINE[+1] + ((int) _PCURLINE[0]*4)) / 8;

#define CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE)	\
	((int) _PCURLINE[-1] + (int) _PCURLINE[+1]) / 2;

#define CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE)	\
	((int) _PPREVLINE[0] + (int) _PNEXTLINE[0]) / 2;

#define CALC_AROUND(_PPREVLINE,_PCURLINE,_PNEXTLINE)	\
	((int) _PCURLINE[-1] + (int) _PCURLINE[+1] + (int) _PPREVLINE[0] + (int) _PNEXTLINE[0]) / 4;

FORCEINLINE byte		edgesensing_ongreen( byte* _PPREVLINE, byte* _PCURLINE, byte* _PNEXTLINE )
{
	unsigned int dH = abs( _PPREVLINE[-1] - _PPREVLINE[+1] );
	unsigned int dV = abs( _PPREVLINE[-1] - _PNEXTLINE[-1] );
	if( 0x07 > dH && 0x07 > dV ) {
		//unsigned int avg = avg(avg(avg(_PPREVLINE[-1],_PPREVLINE[1]),avg(_PNEXTLINE[-1],_PNEXTLINE[1])),_PCURLINE[0]);

		return CALC_DIAGONAL_ongreen( _PPREVLINE, _PCURLINE, _PNEXTLINE );
	} else {
		return _PCURLINE[0];
	}

	//// input : green raster pos
	//if( !(dH > 7 || dV > 7) )
	//{
	//	return _PCURLINE[0];
	//}
	//return CALC_DIAGONAL_ongreen( _PPREVLINE, _PCURLINE, _PNEXTLINE );
}


#define CALC_RGLINE_G(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
	_REF_DEST.r = CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);					\
	_REF_DEST.g = *_PCURLINE;												\
	_REF_DEST.b = CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_RGLINE_G_avg(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
	_REF_DEST.r = CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);					\
	_REF_DEST.g = edgesensing_ongreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);	\
	_REF_DEST.b = CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_RGLINE_R_edge(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
	_REF_DEST.r = *_PCURLINE;											\
	_REF_DEST.g = edgesensing_aroundgreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);	\
	_REF_DEST.b = CALC_DIAGONAL(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_GBLINE_G(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
	_REF_DEST.r = CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);			\
	_REF_DEST.g = *_PCURLINE;												\
	_REF_DEST.b = CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_GBLINE_G_avg(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
	_REF_DEST.r = CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);			\
	_REF_DEST.g = edgesensing_ongreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);	\
	_REF_DEST.b = CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_GBLINE_B_edge(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)	\
	_REF_DEST.b = *_PCURLINE;											\
	_REF_DEST.g = edgesensing_aroundgreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);			\
	_REF_DEST.r = CALC_DIAGONAL(_PPREVLINE,_PCURLINE,_PNEXTLINE);


#endif // BY8_PIXELOPS_H_INC_