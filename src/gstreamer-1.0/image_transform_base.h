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

#ifndef IMAGE_TRANSFORM_BASE_H_INC_
#define IMAGE_TRANSFORM_BASE_H_INC_

#include <stdint.h>

#pragma once

#define CLIP(val,l,h) ( (val) < (l) ? (l) : (val) > (h) ? (h): (val) )

#ifndef mmioFOURCC
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                                \
    ((uint32_t)((ch0) | (ch1)<<8 | (ch2)<<16 | (ch3)<<24))
#endif

#ifndef FOURCC_RGB8
// these are pseudo fourcc's used in the library to signify the formats
#define FOURCC_RGB8         mmioFOURCC( 'R', 'G', 'B', '1' )
#define FOURCC_RGB24        mmioFOURCC( 'R', 'G', 'B', '3' )
#define FOURCC_RGB32        mmioFOURCC( 'R', 'G', 'B', '4' )

#endif

#define FOURCC_YUY2         mmioFOURCC('Y', 'U', 'Y', '2')
#define FOURCC_Y800         mmioFOURCC('Y', '8', '0', '0')
#define FOURCC_BY8          mmioFOURCC('B', 'Y', '8', ' ')
#define FOURCC_UYVY         mmioFOURCC('U', 'Y', 'V', 'Y')

#define FOURCC_YGB0         mmioFOURCC('Y', 'G', 'B', '0')
#define FOURCC_YGB1         mmioFOURCC('Y', 'G', 'B', '1')
#define FOURCC_Y16          mmioFOURCC('Y', '1', '6', ' ')

#define FOURCC_Y444         mmioFOURCC('Y', '4', '4', '4')  // TIYUV: 1394 conferencing camera 4:4:4 mode 0
#define FOURCC_Y411         mmioFOURCC('Y', '4', '1', '1')  // TIYUV: 1394 conferencing camera 4:1:1 mode 2
#define FOURCC_B800         mmioFOURCC('B', 'Y', '8', ' ')
#define FOURCC_Y422         FOURCC_UYVY

#ifndef FOURCC_GRBG
#define FOURCC_GRBG         mmioFOURCC('G', 'R', 'B', 'G')
#endif

#ifndef FOURCC_BGGR8
#define FOURCC_BGGR8        mmioFOURCC('B', 'A', '8', '1') /*  8  BGBG.. GRGR.. */
#define FOURCC_GBRG8        mmioFOURCC('G', 'B', 'R', 'G') /*  8  GBGB.. RGRG.. */
#define FOURCC_GRBG8        mmioFOURCC('G', 'R', 'B', 'G') /*  8  GRGR.. BGBG.. */
#define FOURCC_RGGB8        mmioFOURCC('R', 'G', 'G', 'B') /*  8  RGRG.. GBGB.. */

#endif

#define FOURCC_BGGR10       mmioFOURCC('B', 'G', '1', '0') /* 10  BGBG.. GRGR.. */
#define FOURCC_GBRG10       mmioFOURCC('G', 'B', '1', '0') /* 10  GBGB.. RGRG.. */
#define FOURCC_GRBG10       mmioFOURCC('B', 'A', '1', '0') /* 10  GRGR.. BGBG.. */
#define FOURCC_RGGB10       mmioFOURCC('R', 'G', '1', '0') /* 10  RGRG.. GBGB.. */

#define FOURCC_BGGR12       mmioFOURCC('B', 'G', '1', '2') /* 12  BGBG.. GRGR.. */
#define FOURCC_GBRG12       mmioFOURCC('G', 'B', '1', '2') /* 12  GBGB.. RGRG.. */
#define FOURCC_GRBG12       mmioFOURCC('B', 'A', '1', '2') /* 12  GRGR.. BGBG.. */
#define FOURCC_RGGB12       mmioFOURCC('R', 'G', '1', '2') /* 12  RGRG.. GBGB.. */

#define FOURCC_BGGR16       mmioFOURCC('B', 'G', '1', '6') /* 16  BGBG.. GRGR.. */
#define FOURCC_GBRG16       mmioFOURCC('G', 'B', '1', '6') /* 16  GBGB.. RGRG.. */
#define FOURCC_GRBG16       mmioFOURCC('B', 'A', '1', '6') /* 16  GRGR.. BGBG.. */
#define FOURCC_RGGB16       mmioFOURCC('R', 'G', '1', '6') /* 16  RGRG.. GBGB.. */

#define FOURCC_I420         mmioFOURCC('I', '4', '2', '0')
#define FOURCC_YV16         mmioFOURCC('Y', 'V', '1', '6')      // YUV planar Y plane, U plane, V plane. U and V sub sampled in horz
//#define FOURCC_YV12           mmioFOURCC('Y', 'V', '1', '2')
#define FOURCC_YUV8PLANAR   mmioFOURCC('Y', 'U', '8', 'p')      // unofficial, YUV planar, Y U V planes, all 8 bit, no sub-sampling

#define FOURCC_H264         mmioFOURCC('H', '2', '6', '4')
#define FOURCC_MJPG         mmioFOURCC('M', 'J', 'P', 'G')

#define _MEDIASUBTYPE_FROMFCC( fcc ) { fcc, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } }

typedef unsigned char  byte;

typedef struct
{
    long x;
    long y;
} POINT;


typedef struct
{
    long left;
    long top;
    long right;
    long bottom;
} RECT;


typedef struct
{
    byte*           pData;
    unsigned int    length;

    uint32_t        type;   // this must be at least 32 bit wide

    unsigned int    dim_x;  // pixels
    unsigned int    dim_y;  // lines
    unsigned int    pitch;  // in bytes
} img_descriptor;

#endif // IMAGE_TRANSFORM_BASE_H_INC_
