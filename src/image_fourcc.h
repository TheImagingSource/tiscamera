/*
 * Copyright 2014 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCAM_IMAGE_FOURCC_H
#define TCAM_IMAGE_FOURCC_H

#pragma once

// depends on uint32_t

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
	( (uint32_t)(unsigned char)(ch0) | ( (uint32_t)(unsigned char)(ch1) << 8 ) |    \
	( (uint32_t)(unsigned char)(ch2) << 16 ) | ( (uint32_t)(unsigned char)(ch3) << 24 ) )
#endif

#ifndef FOURCC_RGB8

// these are pseudo FOURCC values used in the library to signify the formats
#define FOURCC_RGB8			    mmioFOURCC( 'R', 'G', 'B', '8' )
#define FOURCC_RGB24		    mmioFOURCC( 'R', 'G', 'B', '3' )
#define FOURCC_RGB32		    mmioFOURCC( 'R', 'G', 'B', '4' )

#endif

// 16 bit per channel
//#define FOURCC_RGB48		    mmioFOURCC( 'R', 'G', 'B', '5' )    // BGR with 16 bit per channel
#define FOURCC_RGB64		    mmioFOURCC( 'R', 'G', 'B', '6' )    // BGRX with 16 bit per channel

#define FOURCC_YUY2			    mmioFOURCC('Y', 'U', 'Y', '2')
#define FOURCC_Y800			    mmioFOURCC('Y', '8', '0', '0')
#define FOURCC_Y12P             mmioFOURCC('Y', '1', '2', 'P')
#define FOURCC_BY8			    mmioFOURCC('B', 'Y', '8', ' ')
#define FOURCC_UYVY			    mmioFOURCC('U', 'Y', 'V', 'Y')
#define FOURCC_YUYV			    mmioFOURCC('Y', 'U', 'Y', 'V')

#define FOURCC_YGB0			    mmioFOURCC('Y', 'G', 'B', '0')
#define FOURCC_YGB1			    mmioFOURCC('Y', 'G', 'B', '1')
#define FOURCC_Y16			    mmioFOURCC('Y', '1', '6', ' ')

#define FOURCC_IYU1             mmioFOURCC('I', 'Y', 'U', '1')      // the same as Y411
#define FOURCC_IYU2             mmioFOURCC('I', 'Y', 'U', '2')      // the same as Y444

#define FOURCC_Y444			    mmioFOURCC('Y', '4', '4', '4')  // TIYUV: 1394 conferencing camera 4:4:4 mode 0, 24 bit
#define FOURCC_Y411			    mmioFOURCC('Y', '4', '1', '1')  // TIYUV: 1394 conferencing camera 4:1:1 mode 2, 12 bit
#define FOURCC_B800			    mmioFOURCC('B', 'Y', '8', ' ')
#define FOURCC_Y422			    FOURCC_UYVY

#define FOURCC_BGGR8		    mmioFOURCC('B', 'A', '8', '1') /*  8  BGBG.. GRGR.. */
#define FOURCC_GBRG8		    mmioFOURCC('G', 'B', 'R', 'G') /*  8  GBGB.. RGRG.. */
#define FOURCC_GRBG8		    mmioFOURCC('G', 'R', 'B', 'G') /*  8  GRGR.. BGBG.. */
#define FOURCC_RGGB8		    mmioFOURCC('R', 'G', 'G', 'B') /*  8  RGRG.. GBGB.. */

#define FOURCC_YUV8			    mmioFOURCC('Y', 'U', 'V', '8')      // 8 bit, U Y V _ ordering, 32 bit

#define FOURCC_I420			    mmioFOURCC('I', '4', '2', '0')      // Y plane, U + V plane sub-sampled horizontally x2, so 2x1
#define FOURCC_YV16			    mmioFOURCC('Y', 'V', '1', '6')		// Y plane, U + V plane sub-sampled horizontally and vertically each x2, so 2x2 sub sampling

// planar formats with no color channel sub-sampling
#define FOURCC_YUV8PLANAR	    mmioFOURCC('Y', 'U', '8', 'p')		// unofficial, YUV planar, Y U V planes, all 8 bit, no sub-sampling
#define FOURCC_YUV16PLANAR	    mmioFOURCC('Y', 'U', 'G', 'p')		// unofficial, YUV planar, Y U V planes, all 16 bit, no sub-sampling
#define FOURCC_YUVFLOATPLANAR	mmioFOURCC('Y', 'U', 'f', 'p')		// unofficial, YUV planar, Y U V planes, all float, no sub-sampling

#define FOURCC_H264			    mmioFOURCC('H', '2', '6', '4')
#define FOURCC_MJPG			    mmioFOURCC('M', 'J', 'P', 'G')


////////////////////////////////////////////////////////////////////////// unused

#define FOURCC_BGGR10		    mmioFOURCC('B', 'G', '1', '0') /* 10  BGBG.. GRGR.. */
#define FOURCC_GBRG10		    mmioFOURCC('G', 'B', '1', '0') /* 10  GBGB.. RGRG.. */
#define FOURCC_GRBG10		    mmioFOURCC('B', 'A', '1', '0') /* 10  GRGR.. BGBG.. */
#define FOURCC_RGGB10		    mmioFOURCC('R', 'G', '1', '0') /* 10  RGRG.. GBGB.. */

#define FOURCC_BGGR12		    mmioFOURCC('B', 'G', '1', '2') /* 12  BGBG.. GRGR.. */
#define FOURCC_GBRG12		    mmioFOURCC('G', 'B', '1', '2') /* 12  GBGB.. RGRG.. */
#define FOURCC_GRBG12		    mmioFOURCC('B', 'A', '1', '2') /* 12  GRGR.. BGBG.. */
#define FOURCC_RGGB12		    mmioFOURCC('R', 'G', '1', '2') /* 12  RGRG.. GBGB.. */

#define FOURCC_BGGR16		    mmioFOURCC('B', 'G', '1', '6') /* 16  BGBG.. GRGR.. */
#define FOURCC_GBRG16		    mmioFOURCC('G', 'B', '1', '6') /* 16  GBGB.. RGRG.. */
#define FOURCC_GRBG16		    mmioFOURCC('B', 'A', '1', '6') /* 16  GRGR.. BGBG.. */
#define FOURCC_RGGB16		    mmioFOURCC('R', 'G', '1', '6') /* 16  RGRG.. GBGB.. */

//////////////////////////////////////////////////////////////////////////


#endif // TCAM_IMAGE_FOURCC_H
