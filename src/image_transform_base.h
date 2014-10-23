
#ifndef IMAGE_TRANSFORM_BASE_H_INC_
#define IMAGE_TRANSFORM_BASE_H_INC_

#pragma once

#include "image_base_defines.h"

#define CLIP(val,l,h) ( (val) < (l) ? (l) : (val) > (h) ? (h): (val) )

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
	( (uint32_t)(unsigned char)(ch0) | ( (uint32_t)(unsigned char)(ch1) << 8 ) |    \
	( (uint32_t)(unsigned char)(ch2) << 16 ) | ( (uint32_t)(unsigned char)(ch3) << 24 ) )
#endif

// these are pseudo fourcc's used in the library to signify the formats
#define FOURCC_RGB8			mmioFOURCC( 'R', 'G', 'B', '8' )
#define FOURCC_RGB24		mmioFOURCC( 'R', 'G', 'B', '3' )
#define FOURCC_RGB32		mmioFOURCC( 'R', 'G', 'B', '4' )
#define FOURCC_RGB64		mmioFOURCC( 'R', 'G', 'B', '5' )

#define FOURCC_YUY2			mmioFOURCC('Y', 'U', 'Y', '2')
#define FOURCC_Y800			mmioFOURCC('Y', '8', '0', '0')
#define FOURCC_BY8			mmioFOURCC('B', 'Y', '8', ' ')
#define FOURCC_UYVY			mmioFOURCC('U', 'Y', 'V', 'Y')

#define FOURCC_YGB0			mmioFOURCC('Y', 'G', 'B', '0')
#define FOURCC_YGB1			mmioFOURCC('Y', 'G', 'B', '1')
#define FOURCC_Y16			mmioFOURCC('Y', '1', '6', ' ')

#define FOURCC_Y444			mmioFOURCC('Y', '4', '4', '4')  // TIYUV: 1394 conferencing camera 4:4:4 mode 0, 24 bit
#define FOURCC_Y411			mmioFOURCC('Y', '4', '1', '1')  // TIYUV: 1394 conferencing camera 4:1:1 mode 2, 12 bit
#define FOURCC_B800			mmioFOURCC('B', 'Y', '8', ' ')
#define FOURCC_Y422			FOURCC_UYVY

#define FOURCC_BGGR8		mmioFOURCC('B', 'A', '8', '1') /*  8  BGBG.. GRGR.. */
#define FOURCC_GBRG8		mmioFOURCC('G', 'B', 'R', 'G') /*  8  GBGB.. RGRG.. */
#define FOURCC_GRBG8		mmioFOURCC('G', 'R', 'B', 'G') /*  8  GRGR.. BGBG.. */
#define FOURCC_RGGB8		mmioFOURCC('R', 'G', 'G', 'B') /*  8  RGRG.. GBGB.. */

#define FOURCC_BGGR10		mmioFOURCC('B', 'G', '1', '0') /* 10  BGBG.. GRGR.. */
#define FOURCC_GBRG10		mmioFOURCC('G', 'B', '1', '0') /* 10  GBGB.. RGRG.. */
#define FOURCC_GRBG10		mmioFOURCC('B', 'A', '1', '0') /* 10  GRGR.. BGBG.. */
#define FOURCC_RGGB10		mmioFOURCC('R', 'G', '1', '0') /* 10  RGRG.. GBGB.. */

#define FOURCC_BGGR12		mmioFOURCC('B', 'G', '1', '2') /* 12  BGBG.. GRGR.. */
#define FOURCC_GBRG12		mmioFOURCC('G', 'B', '1', '2') /* 12  GBGB.. RGRG.. */
#define FOURCC_GRBG12		mmioFOURCC('B', 'A', '1', '2') /* 12  GRGR.. BGBG.. */
#define FOURCC_RGGB12		mmioFOURCC('R', 'G', '1', '2') /* 12  RGRG.. GBGB.. */

#define FOURCC_BGGR16		mmioFOURCC('B', 'G', '1', '6') /* 16  BGBG.. GRGR.. */
#define FOURCC_GBRG16		mmioFOURCC('G', 'B', '1', '6') /* 16  GBGB.. RGRG.. */
#define FOURCC_GRBG16		mmioFOURCC('B', 'A', '1', '6') /* 16  GRGR.. BGBG.. */
#define FOURCC_RGGB16		mmioFOURCC('R', 'G', '1', '6') /* 16  RGRG.. GBGB.. */

#define FOURCC_I420			mmioFOURCC('I', '4', '2', '0')
#define FOURCC_YV16			mmioFOURCC('Y', 'V', '1', '6')		// YUV planar Y plane, U plane, V plane. U and V sub sampled in horz
//#define FOURCC_YV12			mmioFOURCC('Y', 'V', '1', '2')
#define FOURCC_YUV8PLANAR	mmioFOURCC('Y', 'U', '8', 'p')		// unofficial, YUV planar, Y U V planes, all 8 bit, no sub-sampling
#define FOURCC_YUV16PLANAR	mmioFOURCC('Y', 'U', 'h', 'p')		// unofficial, YUV planar, Y U V planes, all 16 bit, no sub-sampling
#define FOURCC_YUV8			mmioFOURCC('Y', 'U', 'V', '8')      // 8 bit, U Y V _ ordering, 32 bit

#define FOURCC_H264			mmioFOURCC('H', '2', '6', '4')
#define FOURCC_MJPG			mmioFOURCC('M', 'J', 'P', 'G')

namespace img
{
	inline bool		isBayerFCC( uint32_t fcc )
	{
		switch( fcc )
		{
		case FOURCC_BY8:		// this is the default
		case FOURCC_BGGR8:
		case FOURCC_GBRG8:
		case FOURCC_RGGB8:
		case FOURCC_GRBG8:
			return true;
		default:
			return false;
		};
	}

	inline int		getBitsPerPixel( uint32_t fcc )
	{
		switch (fcc)
		{
		case FOURCC_RGB24:		return 24;
		case FOURCC_RGB32:		return 32;
		case FOURCC_YUY2:		return 16;
		case FOURCC_UYVY:		return 16;
		case FOURCC_Y800:		return 8;
		case FOURCC_BY8:		return 8;

		case FOURCC_BGGR8:		return 8;
		case FOURCC_GBRG8:		return 8;
		case FOURCC_RGGB8:		return 8;
		case FOURCC_GRBG8:		return 8;

		case FOURCC_YGB0:		return 16;
		case FOURCC_YGB1:		return 16;
		case FOURCC_Y16	 :		return 16;
		case FOURCC_YV16:		return 16;
		case FOURCC_I420:		return 12;
		case FOURCC_YUV8PLANAR:	return 24;

		case FOURCC_BGGR16:		return 16;
		case FOURCC_GBRG16:		return 16;
		case FOURCC_GRBG16:		return 16;
		case FOURCC_RGGB16:		return 16;
		default:
			return 0;
		}
	}

	struct img_type
	{
		uint32_t		type;	// this must be at least 32 bit wide

		unsigned int	dim_x;	// pixels
		unsigned int	dim_y;	// lines

		unsigned int	bytes_per_line;

		unsigned int	buffer_length;
	};

	struct img_descriptor
	{
		byte*			pData;
		unsigned int	length;

		uint32_t		type;	// this must be at least 32 bit wide

		unsigned int	dim_x;	// pixels
		unsigned int	dim_y;	// lines
		unsigned int	pitch;	// in bytes
	};

	inline img_descriptor	to_img_desc( const img_type& t, byte* data, unsigned int size )
	{
		img_descriptor rval = {
			data,
			size,
			t.type,
			t.dim_x,
			t.dim_y,
			t.bytes_per_line
		};
		return rval;
	}
	inline img_descriptor	to_img_desc( const img_type& t, byte* data )
	{
		img_descriptor rval = {
			data,
			t.buffer_length,
			t.type,
			t.dim_x,
			t.dim_y,
			t.bytes_per_line
		};
		return rval;
	}
}

#endif // IMAGE_TRANSFORM_BASE_H_INC_
