
#ifndef BY8_PIXELOPS_H_INC_
#define BY8_PIXELOPS_H_INC_

#pragma once

#include <cmath>

FORCEINLINE uint8_t		edgesensing_aroundgreen( uint8_t* _PPREVLINE, uint8_t* _PCURLINE, uint8_t* _PNEXTLINE ) noexcept
{
    using std::abs;

    const int prev = _PCURLINE[-1];
    const int next = _PCURLINE[+1];
    const int above = _PPREVLINE[0];
    const int below = _PNEXTLINE[0];
    // input : green raster pos
    const int dH = abs( prev - next );
    const int dV = abs( above - below );
    
    if( dH < dV ) {
        return static_cast<uint8_t>((prev + next) / 2);
    }
    else if( dH > dV ) {
        return static_cast<uint8_t>((above + below ) / 2);
    }
    return static_cast<uint8_t>((prev + next + above + below) / 4);
}



#define CALC_DIAGONAL(_PPREVLINE,_PCURLINE,_PNEXTLINE)		\
    (((int) _PPREVLINE[-1] + (int) _PPREVLINE[+1] +			\
    (int) _PNEXTLINE[-1] + (int) _PNEXTLINE[+1]) / 4)

#define CALC_DIAGONAL_ongreen(_PPREVLINE,_PCURLINE,_PNEXTLINE)		\
    (((int) _PPREVLINE[-1] + (int) _PPREVLINE[+1] +			\
    (int) _PNEXTLINE[-1] + (int) _PNEXTLINE[+1] + ((int) _PCURLINE[0]*4)) / 8)

#define CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE)	\
    (((int) _PCURLINE[-1] + (int) _PCURLINE[+1]) / 2)

#define CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE)	\
    (((int) _PPREVLINE[0] + (int) _PNEXTLINE[0]) / 2)

#define CALC_AROUND(_PPREVLINE,_PCURLINE,_PNEXTLINE)	\
    (((int) _PCURLINE[-1] + (int) _PCURLINE[+1] + (int) _PPREVLINE[0] + (int) _PNEXTLINE[0]) / 4)

FORCEINLINE uint8_t		edgesensing_ongreen( uint8_t* _PPREVLINE, uint8_t* _PCURLINE, uint8_t* _PNEXTLINE ) noexcept
{
    using std::abs;

    const int dH = abs( _PPREVLINE[-1] - _PPREVLINE[+1] );
    const int dV = abs( _PPREVLINE[-1] - _PNEXTLINE[-1] );
    if( 0x07 > dH && 0x07 > dV ) {
        //unsigned int avg = avg(avg(avg(_PPREVLINE[-1],_PPREVLINE[1]),avg(_PNEXTLINE[-1],_PNEXTLINE[1])),_PCURLINE[0]);
        const auto res = CALC_DIAGONAL_ongreen( _PPREVLINE, _PCURLINE, _PNEXTLINE );
        return static_cast<uint8_t>(res);
    } else {
        return _PCURLINE[0];
    }
}


#define CALC_RGLINE_G(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
    _REF_DEST.r = (uint8_t)CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);					\
    _REF_DEST.g = *_PCURLINE;												\
    _REF_DEST.b = (uint8_t)CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_RGLINE_G_avg(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
    _REF_DEST.r = (uint8_t)CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);					\
    _REF_DEST.g = edgesensing_ongreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);	\
    _REF_DEST.b = (uint8_t)CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_RGLINE_R_edge(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
    _REF_DEST.r = *_PCURLINE;											\
    _REF_DEST.g = edgesensing_aroundgreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);	\
    _REF_DEST.b = (uint8_t)CALC_DIAGONAL(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_GBLINE_G(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
    _REF_DEST.r = (uint8_t)CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);			\
    _REF_DEST.g = *_PCURLINE;												\
    _REF_DEST.b = (uint8_t)CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_GBLINE_G_avg(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)		\
    _REF_DEST.r = (uint8_t)CALC_OB(_PPREVLINE,_PCURLINE,_PNEXTLINE);			\
    _REF_DEST.g = edgesensing_ongreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);	\
    _REF_DEST.b = (uint8_t)CALC_LR(_PPREVLINE,_PCURLINE,_PNEXTLINE);

#define CALC_GBLINE_B_edge(_PPREVLINE,_PCURLINE,_PNEXTLINE,_REF_DEST)	\
    _REF_DEST.b = *_PCURLINE;											\
    _REF_DEST.g = edgesensing_aroundgreen(_PPREVLINE,_PCURLINE,_PNEXTLINE);			\
    _REF_DEST.r = (uint8_t)CALC_DIAGONAL(_PPREVLINE,_PCURLINE,_PNEXTLINE);


#endif // BY8_PIXELOPS_H_INC_