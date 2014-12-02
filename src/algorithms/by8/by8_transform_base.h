
#ifndef BY8_TRANSFORM_BASE_H_INC_
#define BY8_TRANSFORM_BASE_H_INC_

#pragma once

// this is an internal header for the by8 functions

#include "by8_base.h"

#include <emmintrin.h>              // sse2 intrinsic header, needed for __m128i

#pragma warning ( disable : 4127 ) // conditional expression is constant

//#define BY8_NORGB24_SUPPORT       // currently not working switch
//#define BY8_NOCLRMTX_SUPPORT
//#define BY8_NOAVGGREEN_SUPPORT

namespace by8_transform
{
    using namespace by8_transform::by8_pattern;

    enum output_type
    {
        AO_RGB32 = 0,
        AO_RGB24 = 1,
        AO_YUV8P = 2,
    };

	enum AlgOptions
	{
		AO_UseClrMatrix = 0x1,
		AO_AverageGreen = 0x2,

		AO_AllFlags = 0x03,
	};

    template<unsigned int TParam>
    struct by8_op_settings
    {
#if !defined BY8_NOCLRMTX_SUPPORT
        static const bool	ApplyClrMtx = (TParam & AO_UseClrMatrix) != 0;
#else
        static const bool	ApplyClrMtx = false;
#endif
#if !defined BY8_NOAVGGREEN_SUPPORT
        static const bool	AvgG = (TParam & AO_AverageGreen) != 0;
#else
        static const bool	AvgG = false;
#endif
    };

	typedef struct SSE_ALIGN sse_color_matrix
	{
		enum
		{
            IndexR = 0,
            IndexG = 1,
            IndexB = 2,
		};
		union
		{
			struct
			{
				int16_t r_rfac[8], r_gfac[8], r_bfac[8],
					g_rfac[8], g_gfac[8], g_bfac[8],
					b_rfac[8], b_gfac[8], b_bfac[8];
			};
            struct
            {
                int32_t 
                    r_rfac_epi32[4], r_gfac_epi32[4], r_bfac_epi32[4],
                    g_rfac_epi32[4], g_gfac_epi32[4], g_bfac_epi32[4],
                    b_rfac_epi32[4], b_gfac_epi32[4], b_bfac_epi32[4];
            };
            struct
            {
                __m128i	clr_fac[9];
            };
        };

        union
        {
            struct
            {
                float
                    r_rfac_float[4], r_gfac_float[4], r_bfac_float[4],
                    g_rfac_float[4], g_gfac_float[4], g_bfac_float[4],
                    b_rfac_float[4], b_gfac_float[4], b_bfac_float[4];
            };
            struct
            {
                __m128	clr_fac_float[9];
            };
        };

		int		plane_offset;
	} sse_color_matrix;

	struct alg_context_params : public sse_color_matrix
	{
		unsigned int		alg_options;	// AlgOptions
	};

struct by8_add_packet 
{
	byte*	pPreFirstLine;	// == 0 => pPreFirstLine = pLines - pitch
	byte*	pLines;			// start of the lines to convert. if pPreFirstLine == 0 => pLines - pitch is touched to get the prev line
	byte*	pPostLastLine;

	unsigned int	start_y;	// the start index, relative to all packets for this frame
	unsigned int	line_count;	// the count of lines
	unsigned int	pitch;		// the pitch of the lines in this packet. pitch must be >= the pixel count of the destination buffer
};

struct line_data 
{
	byte*			pPrevLine;
	byte*			pCurLine;
	byte*			pNextLine;

	byte*			pOutLine;
	unsigned int	dim_cx;
};

typedef		void	(* convert_line_function)( const alg_context_params& alg_context, const line_data& line, unsigned int pattern );

void	by8_convert_func( img::img_descriptor& dest, const by8_add_packet& pckt, unsigned int pattern, bool bFlipH, const alg_context_params& alg_context, convert_line_function pFunc );

template<template<unsigned int, output_type> class TFuncGenerate>
inline convert_line_function	find_expansion( unsigned int options, output_type type )
{
    switch( type )
    {
    case AO_RGB32:   // RGB32
        switch( options ) {
        case 0x00:   return TFuncGenerate<0x00, AO_RGB32>::get();
        case 0x01:   return TFuncGenerate<0x01, AO_RGB32>::get();
        case 0x02:   return TFuncGenerate<0x02, AO_RGB32>::get();
        case 0x03:   return TFuncGenerate<0x03, AO_RGB32>::get();
        };
        break;
    case AO_RGB24: // RGB24
        switch( options ) {
        case 0x00:   return TFuncGenerate<0x00, AO_RGB24>::get();
        case 0x01:   return TFuncGenerate<0x01, AO_RGB24>::get();
        case 0x02:   return TFuncGenerate<0x02, AO_RGB24>::get();
        case 0x03:   return TFuncGenerate<0x03, AO_RGB24>::get();
        };
        break;
    case AO_YUV8P: // YUV8P
        switch( options ) {
        case 0x00:   return TFuncGenerate<0x00, AO_YUV8P>::get();
        case 0x01:   return TFuncGenerate<0x01, AO_YUV8P>::get();
        case 0x02:   return TFuncGenerate<0x02, AO_YUV8P>::get();
        case 0x03:   return TFuncGenerate<0x03, AO_YUV8P>::get();
        };
        break;
    };
    return 0;
}

};

#endif // BY8_TRANSFORM_BASE_H_INC_
