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

#include <cstdint>
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
#define FOURCC_BGR24		    mmioFOURCC( 'B', 'G', 'R', '3' )
#define FOURCC_RGB32		    mmioFOURCC( 'R', 'G', 'B', '4' )

#endif

// 16 bit per channel
//#define FOURCC_RGB48		    mmioFOURCC( 'R', 'G', 'B', '5' )    // BGR with 16 bit per channel
#define FOURCC_RGB64		    mmioFOURCC( 'R', 'G', 'B', '6' )    // BGRX with 16 bit per channel

#define FOURCC_YUY2			    mmioFOURCC('Y', 'U', 'Y', '2')
#define FOURCC_Y800			    mmioFOURCC('Y', '8', '0', '0')
// #define FOURCC_Y12P             mmioFOURCC('Y', '1', '2', 'P')
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
#define FOURCC_YUVF32PLANAR         mmioFOURCC('Y', 'U', 'f', 'p')              // unofficial, YUV planar, Y U V planes, all float, no sub-sampling, float range [0.f;1.f] (may be greater for unclipped)
#define FOURCC_YUVFLOATPLANAR	mmioFOURCC('Y', 'U', 'f', 'p')		// unofficial, YUV planar, Y U V planes, all float, no sub-sampling

#define FOURCC_H264			    mmioFOURCC('H', '2', '6', '4')
#define FOURCC_MJPG			    mmioFOURCC('M', 'J', 'P', 'G')


////////////////////////////////////////////////////////////////////////// unused

#define FOURCC_BGGR10		    mmioFOURCC('B', 'G', '1', '0') /* 10  BGBG.. GRGR.. */
#define FOURCC_GBRG10		    mmioFOURCC('G', 'B', '1', '0') /* 10  GBGB.. RGRG.. */
#define FOURCC_GRBG10		    mmioFOURCC('B', 'A', '1', '0') /* 10  GRGR.. BGBG.. */
#define FOURCC_RGGB10		    mmioFOURCC('R', 'G', '1', '0') /* 10  RGRG.. GBGB.. */

/* layout: 0b 00000000'11111111'22222222'33333333'00112233, the upper bits are the low bits of the according pixel
 *		e.g: uint16_t* dst; uint8_t* src; =>  dst[0] = src[0] << 2 | src[4] & 0b11
 */
#define FOURCC_MONO10_MIPI_PACKED		mmioFOURCC('Y', '1', '0', 'P') // Mono, 10-bit
#define FOURCC_GRBG10_MIPI_PACKED		mmioFOURCC('G', 'R', 'A', 'P') // Bayer, 10-bit, BG pattern
#define FOURCC_RGGB10_MIPI_PACKED		mmioFOURCC('R', 'G', 'A', 'P') // Bayer, 10-bit, GB pattern
#define FOURCC_GBRG10_MIPI_PACKED		mmioFOURCC('G', 'B', 'A', 'P') // Bayer, 10-bit, GR pattern
#define FOURCC_BGGR10_MIPI_PACKED		mmioFOURCC('B', 'G', 'A', 'P') // Bayer, 10-bit, RG pattern

/* layout: 0b 00000000'00111111'11112222'22222233'33333333
 *		e.g: uint16_t* dst; uint8_t* src; dst[0] = src[0] | (src[1] & 0b11) << 2
 */
#define FOURCC_MONO10_SPACKED	mmioFOURCC('Y', '1', '0', 'p') // Mono, 10-bit
#define FOURCC_GRBG10_SPACKED	mmioFOURCC('G', 'R', 'A', 'p') // Bayer, 10-bit, BG pattern
#define FOURCC_RGGB10_SPACKED	mmioFOURCC('R', 'G', 'A', 'p') // Bayer, 10-bit, GB pattern
#define FOURCC_GBRG10_SPACKED	mmioFOURCC('G', 'B', 'A', 'p') // Bayer, 10-bit, GR pattern
#define FOURCC_BGGR10_SPACKED	mmioFOURCC('B', 'G', 'A', 'p') // Bayer, 10-bit, RG pattern

#define FOURCC_BGGR12		    mmioFOURCC('B', 'G', '1', '2') /* 12  BGBG.. GRGR.. */
#define FOURCC_GBRG12		    mmioFOURCC('G', 'B', '1', '2') /* 12  GBGB.. RGRG.. */
#define FOURCC_GRBG12		    mmioFOURCC('B', 'A', '1', '2') /* 12  GRGR.. BGBG.. */
#define FOURCC_RGGB12		    mmioFOURCC('R', 'G', '1', '2') /* 12  RGRG.. GBGB.. */

#define FOURCC_GRBG12_SPACKED   mmioFOURCC('G', 'R', 'C', 'p') /* 12  u8, [pix0_lo][pix0_hi | pix1_lo][pix1_hi] */
#define FOURCC_RGGB12_SPACKED   mmioFOURCC('R', 'G', 'C', 'p') /* 12   */
#define FOURCC_GBRG12_SPACKED   mmioFOURCC('G', 'B', 'C', 'p') /* 12   */
#define FOURCC_BGGR12_SPACKED   mmioFOURCC('B', 'G', 'C', 'p') /* 12   */

#define FOURCC_GRBG12_PACKED    mmioFOURCC('G', 'R', 'C', 'P') /* 12  u8, [pix0_hi][pix0_lo | pix1_lo][pix1_hi] */
#define FOURCC_RGGB12_PACKED    mmioFOURCC('R', 'G', 'C', 'P') /* 12   */
#define FOURCC_GBRG12_PACKED    mmioFOURCC('G', 'B', 'C', 'P') /* 12   */
#define FOURCC_BGGR12_PACKED    mmioFOURCC('B', 'G', 'C', 'P') /* 12   */

#define FOURCC_GRBG12_MIPI_PACKED       mmioFOURCC('G', 'R', 'D', 'P') /* 12  u8, [pix0_hi][pix1_hi][pix0_lo | pix1_lo] */
#define FOURCC_RGGB12_MIPI_PACKED       mmioFOURCC('R', 'G', 'D', 'P') /* 12   */
#define FOURCC_GBRG12_MIPI_PACKED       mmioFOURCC('G', 'B', 'D', 'P') /* 12   */
#define FOURCC_BGGR12_MIPI_PACKED       mmioFOURCC('B', 'G', 'D', 'P') /* 12   */

#define FOURCC_BGGR16		    mmioFOURCC('B', 'G', '1', '6') /* 16  BGBG.. GRGR.. */
#define FOURCC_GBRG16		    mmioFOURCC('G', 'B', '1', '6') /* 16  GBGB.. RGRG.. */
#define FOURCC_GRBG16		    mmioFOURCC('B', 'A', '1', '6') /* 16  GRGR.. BGBG.. */
#define FOURCC_RGGB16		    mmioFOURCC('R', 'G', '1', '6') /* 16  RGRG.. GBGB.. */

//////////////////////////////////////////////////////////////////////////

#define FOURCC_Y10_PACKED                   mmioFOURCC('Y', '1', '0', 'p') /* 10  u8, 5 pix, 4 bytes */
#define FOURCC_Y12_SPACKED                  mmioFOURCC('Y', '1', '2', 'p') /* 12  u8, [pix0_hi][pix0_lo | pix1_hi][pix1_lo] */
#define FOURCC_Y12_PACKED                   mmioFOURCC('Y', '1', '2', 'P') /* 12  u8, [pix0_hi][pix0_lo | pix1_lo][pix1_hi] */
#define FOURCC_Y12_MIPI_PACKED          mmioFOURCC('Y', '1', 'D', 'P') /* 12  u8, [pix0_hi][pix1_hi][pix0_lo | pix1_lo] */

// Compression format used by GigE3L

#define FOURCC_BY8_GR_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '0' )
#define FOURCC_BY8_RG_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '1' )
#define FOURCC_BY8_GB_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '2' )
#define FOURCC_BY8_BG_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '3' )


#define FOURCC_Y800_NIBBLE_RLE_COMPRESSED       mmioFOURCC( 'C', 'Y', '6', '0' )


// Image Polarization format

// We format Polarizations formats via prefix 'PXYY', 'P' is prefix, X is the Filter polarization angle
// YY is the bit depth, E.g. '80' for 8 bit, 'Cp' for 12 bit packed, 'CP' for 16 bit packed, and '16' bit packed
// X stands for the filter polarization angles, e.g. 1 ^= line + 0 [P90][P45], line + 1 [P135][P0]

#define FOURCC_POLARIZATION_MONO8_90_45_135_0           mmioFOURCC('P', '1', '8', '0')   // Polarization format 8-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_MONO16_90_45_135_0          mmioFOURCC('P', '1', '1', '6')   // Polarization format 16-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_MONO12_90_45_135_0          mmioFOURCC('P', '1', 'C', 'p')   // Polarization format 16-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_MONO12_SPACKED_90_45_135_0  mmioFOURCC('P', '1', 'C', 'p') // Polarization format 12-bit SPACKED, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_MONO12_PACKED_90_45_135_0   mmioFOURCC('P', '1', 'C', 'P')   // Polarization format 12-bit PACKED, line + 0 [P90][P45], line + 1 [P135][P0]

#define FOURCC_POLARIZATION_BAYER_BG8_90_45_135_0             mmioFOURCC('P', '2', '8', '0')  // Polarization format 8-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_BAYER_BG12_90_45_135_0      mmioFOURCC('P', '2', 'C', 'p')   // Polarization format 16-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_BAYER_BG16_90_45_135_0            mmioFOURCC('P', '2', '1', '6')  // Polarization format 16-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_BAYER_BG12_SPACKED_90_45_135_0    mmioFOURCC('P', '2', 'C', 'p') // Polarization format 12-bit SPACKED, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_BAYER_BG12_PACKED_90_45_135_0     mmioFOURCC('P', '2', 'C', 'P')   // Polarization format 12-bit PACKED, line + 0 [P90][P45], line + 1 [P135][P0]

// internal ADI formats
#define FOURCC_POLARIZATION_ADI_PLANAR_MONO8            mmioFOURCC('A', 'D', 'p', '1')  // Polarization ADI planar image, uint8_t planes [angleOfMaxPolarization, linearityOfPolarization, intensity, unused]
#define FOURCC_POLARIZATION_ADI_PLANAR_MONO16           mmioFOURCC('A', 'D', 'p', '2')  // Polarization ADI planar image, uint16_t planes [angleOfMaxPolarization, linearityOfPolarization, intensity, unused]

// output polarization formats
#define FOURCC_POLARIZATION_ADI_MONO8                   mmioFOURCC('A', 'D', 'I', '1')  // Polarization result image, packed uint8_t  [angleOfMaxPolarization, linearityOfPolarization, intensity, unused]
#define FOURCC_POLARIZATION_ADI_MONO16                  mmioFOURCC('A', 'D', 'I', '2')  // Polarization result image, packed uint16_t [angleOfMaxPolarization, linearityOfPolarization, intensity, unused]
#define FOURCC_POLARIZATION_ADI_RGB8                    mmioFOURCC('A', 'D', 'C', '1')  // Polarization result image, packed uint8_t  [angleOfMaxPolarization, linearityOfPolarization, R0, G0, B0, R1, G1, B1, all 8 bit
#define FOURCC_POLARIZATION_ADI_RGB16                   mmioFOURCC('A', 'D', 'C', '2')  // Polarization result image, packed uint16_t [angleOfMaxPolarization, linearityOfPolarization, R0, G0, B0, R1, G1, B1, all 8 bit


#define FOURCC_POLARIZATION_PACKED8                     mmioFOURCC('P', 'P', 'M', '1')  // Polarization result image, packed uint8_t [0;45;90;135] degrees, pixel ^= 32 bit
#define FOURCC_POLARIZATION_PACKED16                    mmioFOURCC('P', 'P', 'M', '2')  // Polarization result image, packed uint8_t [0;45;90;135] degrees, pixel ^= 32 bit
#define FOURCC_POLARIZATION_PACKED8_BAYER_BG            mmioFOURCC('P', 'P', 'B', '1')  // Polarization result image, packed uint16_t [0;45;90;135] degrees, pixel ^= 64 bit
#define FOURCC_POLARIZATION_PACKED16_BAYER_BG           mmioFOURCC('P', 'P', 'B', '2')  // Polarization result image, packed uint16_t [0;45;90;135] degrees, pixel ^= 64 bit

// PWL formats:
#define FOURCC_PWL_RG12_MIPI							mmioFOURCC('P', 'W', 'L', '1')  // PWL format, RGGB 12-bit, MIPI-packed
#define FOURCC_PWL_RG12									mmioFOURCC('P', 'W', 'L', '2')  // PWL format, RGGB 16-bit, [0,0xXFFF] (X ^= 0 or ignored)
#define FOURCC_PWL_RG16H12								mmioFOURCC('P', 'W', 'L', '3')  // PWL format, RGGB 16-bit, [0,0xFFFX]


#endif // TCAM_IMAGE_FOURCC_H
