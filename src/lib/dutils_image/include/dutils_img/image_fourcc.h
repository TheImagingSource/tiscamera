
#ifndef IMAGE_FOURCC_H_INC_
#define IMAGE_FOURCC_H_INC_

#pragma once

#include <cstdint>  // uint32_t

#ifndef mmioFOURCC    
#define mmioFOURCC( ch0, ch1, ch2, ch3 )                \
	( (uint32_t)(unsigned char)(ch0) | ( (uint32_t)(unsigned char)(ch1) << 8 ) |    \
	( (uint32_t)(unsigned char)(ch2) << 16 ) | ( (uint32_t)(unsigned char)(ch3) << 24 ) )
#endif

/* When adding a new type, you have to also add it to several places to get recognized ...
	- A FCC definition here
	- image_fourcc_enum.h 
	- image_fourcc_func.h get_bits_per_pixel
	- fcc_to_string.h
	- image_fourcc_func.h to add to a category or image_bayer_pattern.h to add to a bayer format mapping

 */

// these are pseudo FOURCC values used in the library to signify the formats
#define FOURCC_BGR24		    mmioFOURCC( 'B', 'G', 'R', '3' )	// BGR, channel: 8-bit, struct: 24-bit
#define FOURCC_BGRA32		    mmioFOURCC( 'B', 'G', 'R', '4' )	// BGRx, channel: 8-bit, struct: 32-bit
#define FOURCC_BGRA64		    mmioFOURCC( 'R', 'G', 'B', '6' )    // BGRx, channel: 16-bit, little endian, struct: 64-bit. Note that the FourCC is 'RGB6' and this cannot change!!!!

#define FOURCC_BGRFloat			mmioFOURCC( 'B', 'G', 'r', 'f' )    // BGR, channel: 32-bit float, little endian, struct: 96-bit

#define FOURCC_RAW8				mmioFOURCC( 'R', 'A', 'W', '1' )	// 8-bit
#define FOURCC_RAW16			mmioFOURCC( 'R', 'A', 'W', '2' )	// 16-bit
#define FOURCC_RAW24			mmioFOURCC( 'R', 'A', 'W', '3' )	// 24-bit
#define FOURCC_RAW32			mmioFOURCC( 'R', 'A', 'W', '4' )	// 32-bit

#define FOURCC_RAWFloat			mmioFOURCC( 'R', 'A', 'W', 'f' )	// float, little endian


#define FOURCC_HSV24		    mmioFOURCC( 'H', 'S', 'V', '3' )    // HSV, channel: 8-bit, struct: 24-bit
#define FOURCC_HSVx32		    mmioFOURCC( 'H', 'S', 'V', '4' )    // HSVx, channel: 8-bit, struct: 32-bit, x channel => unused

// YUV formats
#define FOURCC_YUY2			    mmioFOURCC('Y', 'U', 'Y', '2')		// YUYV, channel: 8-bit, effective bit depth 16-bit
#define FOURCC_UYVY			    mmioFOURCC('U', 'Y', 'V', 'Y')		// UYVY, channel: 8-bit, effective bit depth 16-bit
#define FOURCC_I420			    mmioFOURCC('I', '4', '2', '0')      // Y plane, U + V plane sub-sampled 2x2
//#define FOURCC_YV16			    mmioFOURCC('Y', 'V', '1', '6')		// Y plane, U + V plane sub-sampled 2x1
#define FOURCC_IYU1             mmioFOURCC('I', 'Y', 'U', '1')      // the same as Y411
#define FOURCC_IYU2             mmioFOURCC('I', 'Y', 'U', '2')      // the same as Y444
#define FOURCC_Y411			    mmioFOURCC('Y', '4', '1', '1')      // TIYUV: 1394 conferencing camera 4:1:1 mode 2, 12 bit
#define FOURCC_NV12				mmioFOURCC('N', 'V', '1', '2')      // YUV 4:1:1 format, Y Plane, Interleaved 2x2 subsampled V/U Plane, 12 bit  see https://www.fourcc.org/pixel-format/yuv-nv12/
#define FOURCC_YV12				mmioFOURCC('Y', 'V', '1', '2')      // YUV 4:1:1 format, Y Plane, 2x2 subsampled V plane, 2x2 subcampled U Plane, 12 bit  see https://www.fourcc.org/pixel-format/yuv-yv12/

//#define FOURCC_Y444			    mmioFOURCC('Y', '4', '4', '4')      // TIYUV: 1394 conferencing camera 4:4:4 mode 0, 24 bit
//#define FOURCC_Y422			    FOURCC_UYVY
//#define FOURCC_YUV8			    mmioFOURCC('Y', 'U', 'V', '8')      // 8 bit, U Y V _ ordering, 32 bit

#define FOURCC_BY8_dead			mmioFOURCC('B', 'Y', '8', ' ')      // do not use!!
#define FOURCC_YGB0_dead		mmioFOURCC('Y', 'G', 'B', '0')      // do not use!!
#define FOURCC_YGB1_dead		mmioFOURCC('Y', 'G', 'B', '1')      // do not use!!

// Mono formats, NOTE: because windows uses FOURCC_Y800 + FOURCC_Y16 in DShow for MonoX and for RawX these are somewhat bad !!
#define FOURCC_Y800			    mmioFOURCC('Y', '8', '0', '0')		// Mono, 8-bit
#define FOURCC_Y16			    mmioFOURCC('Y', '1', '6', ' ')		// Mono, 16-bit, little endian, [0;0xFFFF]

#define FOURCC_MONO8			FOURCC_Y800							// Mono, 8-bit
#define FOURCC_MONO10			mmioFOURCC('Y', '1', '0', ' ')		// Mono, 16-bit, little endian, [0;0x03FF]
#define FOURCC_MONO12			mmioFOURCC('Y', '1', '2', ' ')		// Mono, 16-bit, little endian, [0;0x0FFF]
#define FOURCC_MONO16			FOURCC_Y16							// Mono, 16-bit, little endian, [0;0xFFFF]
#define FOURCC_MONOFloat		mmioFOURCC('M', 'O', 'N', 'f')		// Mono, 32-bit float, little endian

// Bayer formats
#define FOURCC_BGGR8		    mmioFOURCC('B', 'A', '8', '1')		// Bayer, 8-bit, BG pattern
#define FOURCC_GBRG8		    mmioFOURCC('G', 'B', 'R', 'G')		// Bayer, 8-bit, GB pattern
#define FOURCC_GRBG8		    mmioFOURCC('G', 'R', 'B', 'G')		// Bayer, 8-bit, GR pattern
#define FOURCC_RGGB8		    mmioFOURCC('R', 'G', 'G', 'B')		// Bayer, 8-bit, RG pattern

#define FOURCC_BGGR16		    mmioFOURCC('B', 'G', '1', '6')		// Bayer, 16-bit, [0;0xFFFF], little endian, BG pattern
#define FOURCC_GBRG16		    mmioFOURCC('G', 'B', '1', '6')		// Bayer, 16-bit, [0;0xFFFF], little endian, GB pattern
#define FOURCC_GRBG16		    mmioFOURCC('B', 'A', '1', '6')		// Bayer, 16-bit, [0;0xFFFF], little endian, GR pattern
#define FOURCC_RGGB16		    mmioFOURCC('R', 'G', '1', '6')		// Bayer, 16-bit, [0;0xFFFF], little endian, RG pattern

#define FOURCC_BGGR10			mmioFOURCC('B', 'G', '1', '0')		// Bayer, 16-bit, [0;0x03FF], little endian, BG pattern
#define FOURCC_GBRG10			mmioFOURCC('G', 'B', '1', '0')		// Bayer, 16-bit, [0;0x03FF], little endian, GB pattern
#define FOURCC_GRBG10			mmioFOURCC('B', 'A', '1', '0')		// Bayer, 16-bit, [0;0x03FF], little endian, GR pattern
#define FOURCC_RGGB10			mmioFOURCC('R', 'G', '1', '0')		// Bayer, 16-bit, [0;0x03FF], little endian, RG pattern

#define FOURCC_BGGR12			mmioFOURCC('B', 'G', '1', '2')		// Bayer, 16-bit, [0;0x0FFF], little endian, BG pattern
#define FOURCC_GBRG12			mmioFOURCC('G', 'B', '1', '2')		// Bayer, 16-bit, [0;0x0FFF], little endian, GB pattern
#define FOURCC_GRBG12			mmioFOURCC('B', 'A', '1', '2')		// Bayer, 16-bit, [0;0x0FFF], little endian, GR pattern
#define FOURCC_RGGB12			mmioFOURCC('R', 'G', '1', '2')		// Bayer, 16-bit, [0;0x0FFF], little endian, RG pattern

#define FOURCC_BGGRFloat		    mmioFOURCC('B', 'G', 'f', '0')		// Bayer, 32-bit float, little endian, BG pattern
#define FOURCC_GBRGFloat		    mmioFOURCC('G', 'B', 'f', '0')		// Bayer, 32-bit float, little endian, BG pattern
#define FOURCC_GRBGFloat		    mmioFOURCC('B', 'A', 'f', '0')		// Bayer, 32-bit float, little endian, BG pattern
#define FOURCC_RGGBFloat		    mmioFOURCC('R', 'G', 'f', '0')		// Bayer, 32-bit float, little endian, BG pattern


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

/* layout: bits => 00000000 00001111 11111111
 */
#define FOURCC_GRBG12_SPACKED	mmioFOURCC('G', 'R', 'C', 'p')  // Bayer, 12-bit, BG pattern, [pix0_lo][pix0_hi | pix1_hi][pix1_lo]
#define FOURCC_RGGB12_SPACKED	mmioFOURCC('R', 'G', 'C', 'p')  // Bayer, 12-bit, GB pattern
#define FOURCC_GBRG12_SPACKED	mmioFOURCC('G', 'B', 'C', 'p')  // Bayer, 12-bit, GR pattern
#define FOURCC_BGGR12_SPACKED	mmioFOURCC('B', 'G', 'C', 'p')  // Bayer, 12-bit, RG pattern

 /* layout: bits => 00000000 00001111 11111111, [pix0_hi][pix0_lo | pix1_lo][pix1_hi]
  */
#define FOURCC_GRBG12_PACKED	mmioFOURCC('G', 'R', 'C', 'P') // Bayer, 12-bit, BG pattern, [pix0_hi][pix0_lo | pix1_lo][pix1_hi]
#define FOURCC_RGGB12_PACKED	mmioFOURCC('R', 'G', 'C', 'P') // Bayer, 12-bit, GB pattern
#define FOURCC_GBRG12_PACKED	mmioFOURCC('G', 'B', 'C', 'P') // Bayer, 12-bit, GR pattern
#define FOURCC_BGGR12_PACKED	mmioFOURCC('B', 'G', 'C', 'P') // Bayer, 12-bit, RG pattern

/* layout: bits => 00000000 11111111 00001111, [pix0_hi][pix1_hi][pix0_lo | pix1_lo]
 */
#define FOURCC_GRBG12_MIPI_PACKED	mmioFOURCC('G', 'R', 'D', 'P') // Bayer, 12-bit, BG pattern, [pix0_hi][pix1_hi][pix0_lo | pix1_lo]
#define FOURCC_RGGB12_MIPI_PACKED	mmioFOURCC('R', 'G', 'D', 'P') // Bayer, 12-bit, GB pattern
#define FOURCC_GBRG12_MIPI_PACKED	mmioFOURCC('G', 'B', 'D', 'P') // Bayer, 12-bit, GR pattern
#define FOURCC_BGGR12_MIPI_PACKED	mmioFOURCC('B', 'G', 'D', 'P') // Bayer, 12-bit, RG pattern

// Mono packed formats:
#define FOURCC_MONO12_PACKED		    mmioFOURCC('Y', '1', '2', 'P') // Mono, 12-bit, layout ^= FOURCC_GRBG12_PACKED
#define FOURCC_MONO12_SPACKED		    mmioFOURCC('Y', '1', '2', 'p') // Mono, 12-bit, layout ^= FOURCC_GRBG12_SPACKED
#define FOURCC_MONO12_MIPI_PACKED		mmioFOURCC('Y', '1', 'D', 'P') // Mono, 12-bit, layout ^= FOURCC_GRBG12_MIPI_PACKED

// Planar formats with no color channel sub-sampling
#define FOURCC_YUV8PLANAR	    mmioFOURCC('Y', 'U', '8', 'p')		// internal, YUV planar, Y U V planes, all 8 bit, no sub-sampling
#define FOURCC_YUV16PLANAR	    mmioFOURCC('Y', 'U', 'G', 'p')		// internal, YUV planar, Y U V planes, 16 bit, little endian, no sub-sampling
#define FOURCC_YUVF32PLANAR	    mmioFOURCC('Y', 'U', 'f', 'p')		// internal, YUV planar, Y U V planes, all float, no sub-sampling, float range [0.f;1.f] (may be greater for unclipped)

// Compression formats, used by GigE3L
#define FOURCC_BY8_GR_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '0' ) // deprecated
#define FOURCC_BY8_RG_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '1' ) // deprecated
#define FOURCC_BY8_GB_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '2' ) // deprecated
#define FOURCC_BY8_BG_NIBBLE_RLE_COMPRESSED     mmioFOURCC( 'C', 'B', 'Y', '3' ) // deprecated

#define FOURCC_MONO8_NIBBLE_RLE_COMPRESSED       mmioFOURCC( 'C', 'Y', '6', '0' ) // deprecated

// compressed formats
//#define FOURCC_H264			    mmioFOURCC('H', '2', '6', '4') 
#define FOURCC_MJPG			    mmioFOURCC('M', 'J', 'P', 'G') 

// Image Polarization format

// We format Polarizations formats via prefix 'PXYY', 'P' is prefix, X is the Filter polarization angle
// YY is the bit depth, E.g. '80' for 8 bit, 'Cp' for 12 bit packed, 'CP' for 16 bit packed, and '16' bit packed
// X stands for the filter polarization angles, e.g. 1 ^= line + 0 [P90][P45], line + 1 [P135][P0]

#define FOURCC_POLARIZATION_MONO8_90_45_135_0           mmioFOURCC('P', '1', '8', '0')   // Polarization format 8-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_MONO16_90_45_135_0          mmioFOURCC('P', '1', '1', '6')   // Polarization format 16-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_MONO12_SPACKED_90_45_135_0  mmioFOURCC('P', '1', 'C', 'p') // Polarization format 12-bit SPACKED, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_MONO12_PACKED_90_45_135_0   mmioFOURCC('P', '1', 'C', 'P')   // Polarization format 12-bit PACKED, line + 0 [P90][P45], line + 1 [P135][P0]

#define FOURCC_POLARIZATION_BG8_90_45_135_0             mmioFOURCC('P', '2', '8', '0')  // Polarization format 8-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_BG16_90_45_135_0            mmioFOURCC('P', '2', '1', '6')  // Polarization format 16-bit, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_BG12_SPACKED_90_45_135_0    mmioFOURCC('P', '2', 'C', 'p') // Polarization format 12-bit SPACKED, line + 0 [P90][P45], line + 1 [P135][P0]
#define FOURCC_POLARIZATION_BG12_PACKED_90_45_135_0     mmioFOURCC('P', '2', 'C', 'P')   // Polarization format 12-bit PACKED, line + 0 [P90][P45], line + 1 [P135][P0]

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

#endif // IMAGE_FOURCC_H_INC_