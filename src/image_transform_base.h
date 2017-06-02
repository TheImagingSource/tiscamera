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

#ifndef TCAM_IMAGE_TRANSFORM_BASE_H
#define TCAM_IMAGE_TRANSFORM_BASE_H

#pragma once

#include "image_base_defines.h"
#include "image_fourcc.h"

#ifndef CLIP
#define CLIP(val,l,h) ( (val) < (l) ? (l) : (val) > (h) ? (h): (val) )
#endif

namespace img
{
inline int get_bits_per_pixel (uint32_t fcc)
{
    switch (fcc)
    {
        case FOURCC_RGB24:      return 24;
        case FOURCC_BGR24:      return 24;
        case FOURCC_RGB32:      return 32;
        case FOURCC_YUY2:       return 16;
        case FOURCC_UYVY:       return 16;
        case FOURCC_YUYV:       return 16;
        case FOURCC_Y800:       return 8;
        case FOURCC_BY8:        return 8;

        case FOURCC_BGGR8:      return 8;
        case FOURCC_GBRG8:      return 8;
        case FOURCC_RGGB8:      return 8;
        case FOURCC_GRBG8:      return 8;

        case FOURCC_YGB0:       return 16;
        case FOURCC_YGB1:       return 16;
        case FOURCC_Y16:        return 16;
        case FOURCC_YV16:       return 16;
        case FOURCC_I420:       return 12;
        case FOURCC_YUV8PLANAR: return 24;

        case FOURCC_BGGR16:     return 16;
        case FOURCC_GBRG16:     return 16;
        case FOURCC_GRBG16:     return 16;
        case FOURCC_RGGB16:     return 16;
        case FOURCC_Y411:
        case FOURCC_IYU1:       return 14;      // beware
        case FOURCC_IYU2:       return 24;

        //case FOURCC_RGB48:          return 48;
        case FOURCC_RGB8:           return 8;
        case FOURCC_RGB64:          return 64;

        case FOURCC_YUV16PLANAR:    return 48;
        case FOURCC_YUVFLOATPLANAR: return 96;
        case FOURCC_MJPG: return 16;
        default:
            return 0;
    }
}

    inline bool    is_known_fcc( uint32_t fcc )
    {
        return get_bits_per_pixel( fcc ) != 0;
    }


    inline int      calc_minimum_pitch( uint32_t fcc, unsigned dim_x, int bpp )
    {
        switch( fcc )
        {
        case FOURCC_YV16:		    return dim_x * 1;       // these are sub-sampled in u and v, so specify the y plane pitch
        case FOURCC_I420:		    return dim_x * 1;       // these are sub-sampled in u and v, so specify the y plane pitch

        case FOURCC_YUV8PLANAR:	    return dim_x * 1;
        case FOURCC_YUV16PLANAR:    return dim_x * 2;
        case FOURCC_YUVFLOATPLANAR: return dim_x * 4;
        default:
            return (dim_x * bpp) / 8;
        }
    }

    inline int      calc_minimum_pitch( uint32_t fcc, unsigned dim_x )
    {
        switch( fcc )
        {
        case FOURCC_YV16:		    return dim_x * 1;       // these are sub-sampled in u and v, so specify the y plane pitch
        case FOURCC_I420:		    return dim_x * 1;       // these are sub-sampled in u and v, so specify the y plane pitch

        case FOURCC_YUV8PLANAR:	    return dim_x * 1;
        case FOURCC_YUV16PLANAR:    return dim_x * 2;
        case FOURCC_YUVFLOATPLANAR: return dim_x * 4;
        default:
            return (dim_x * get_bits_per_pixel( fcc )) / 8;
        }
    }
    inline int      calc_img_size_from_pitch( uint32_t fcc, int pitch, unsigned dim_y )
    {
        unsigned bytes_per_line = pitch < 0 ? -pitch : pitch;
        switch( fcc )
        {
        case FOURCC_YV16:		    return (bytes_per_line * dim_y) + (bytes_per_line * dim_y / 2);     // u and v plane are sub-sampled 2x1
        case FOURCC_I420:		    return (bytes_per_line * dim_y) + (bytes_per_line * dim_y / 4);     // u and v plane are sub-sampled 2x2

        case FOURCC_YUV8PLANAR:	    return bytes_per_line * dim_y * 3;
        case FOURCC_YUV16PLANAR:    return bytes_per_line * dim_y * 3;
        case FOURCC_YUVFLOATPLANAR: return bytes_per_line * dim_y * 3;
        default:
            return bytes_per_line * dim_y;
        }
    }

    inline int      calc_minimum_img_size( uint32_t fcc, unsigned dim_x, unsigned dim_y )
    {
        return calc_img_size_from_pitch( fcc, calc_minimum_pitch( fcc, dim_x ), dim_y );
    }

    inline bool     is_multi_plane_format( uint32_t fcc )
    {
        switch( fcc )
        {
        case FOURCC_YV16:		    return true;       // these are sub-sampled in u and v, so specify the y plane pitch
        case FOURCC_I420:		    return true;       // these are sub-sampled in u and v, so specify the y plane pitch

        case FOURCC_YUV8PLANAR:	    return true;
        case FOURCC_YUV16PLANAR:    return true;
        case FOURCC_YUVFLOATPLANAR: return true;
        default:
            return false;
        }
    }


	struct img_type
	{
		uint32_t		type;

		unsigned int	dim_x;	            // pixels
		unsigned int	dim_y;	            // lines

		unsigned int	bytes_per_line;     // this is the stride to get from one line to the next, this may be > then dim_x * get_bits_per_pixel( type )

		unsigned int	buffer_length;      // this is the length of the buffer to be allocated for the full image
	};

	struct img_descriptor
	{
		uint32_t		type;	// this must be at least 32 bit wide

		unsigned int	dim_x;	// pixels
		unsigned int	dim_y;	// lines
		int	            pitch;	// in bytes, this is the count of bytes it takes from the start of one line to get to the next line
                                // for planar formats this contains the count of bytes to get from one line in one plane to the next line in the same
                                // note that this can be negative

        unsigned int	data_length;    // the actual data length valid in this buffer. this is in most cases calc_img_size( type, dim_x, dim_y ), but may be smaller for e.g. compressed formats

        byte*			pData;  // this is the pointer of the 'first' byte of the image. this can be the first byte of the last line if pitch is negative
	};




    inline int      calc_plane_pitch( const img_descriptor& dsc )
    {
        ASSERT( dsc.type == FOURCC_YUV16PLANAR || dsc.type == FOURCC_YUV8PLANAR || dsc.type == FOURCC_YUVFLOATPLANAR );
        int pitch = dsc.pitch < 0 ? -dsc.pitch : dsc.pitch;
        return (int)dsc.dim_y * pitch;
    }

    inline img_type make_img_type( uint32_t fcc, unsigned width, unsigned height, unsigned bytes_per_line, unsigned size )
    {
        img::img_type rval = { fcc, width, height, bytes_per_line, size};
        return rval;
    }
    inline img_type make_img_type( uint32_t fcc, unsigned width, unsigned height, unsigned bytes_per_line )
    {
        return make_img_type( fcc, width, height, bytes_per_line, calc_img_size_from_pitch( fcc, bytes_per_line, height ) );
    }
    inline img_type make_img_type( uint32_t fcc, unsigned width, unsigned height )
    {
        int bpp = img::get_bits_per_pixel( fcc );
        ASSERT( bpp != 0 );
        int pitch = calc_minimum_pitch( fcc, width, bpp );
        return make_img_type( fcc, width, height, pitch, calc_img_size_from_pitch( fcc, pitch, height ) );
    }

    inline img_type make_img_type_with_bpp( uint32_t fcc, unsigned width, unsigned height, unsigned bpp )
    {
        int pitch = calc_minimum_pitch( fcc, width, bpp );
        return make_img_type( fcc, width, height, pitch, calc_img_size_from_pitch( fcc, pitch, height ) );
    }

    inline img_descriptor	to_img_desc( void* data, uint32_t fcc, unsigned width, unsigned height, int pitch, unsigned size )
    {
        img_descriptor rval = {
            fcc,
            width,
            height,
            pitch,
            size,
            reinterpret_cast<byte*>(data),
        };
        return rval;
    }
	inline img_descriptor	to_img_desc( const img_type& t, void* data, unsigned int size )
	{
		return to_img_desc( data, t.type, t.dim_x, t.dim_y, t.bytes_per_line, size );
	}
	inline img_descriptor	to_img_desc( const img_type& t, void* data )
	{
        return to_img_desc( data, t.type, t.dim_x, t.dim_y, t.bytes_per_line, t.buffer_length );
	}

    inline img_descriptor	copy_img_desc( const img_descriptor& desc, uint32_t new_fcc )
    {
        return to_img_desc( desc.pData, new_fcc, desc.dim_x, desc.dim_y, desc.pitch, desc.data_length );
    }

    inline img_descriptor  flip_image_in_img_desc( const img_descriptor& dsc )
    {
        if( dsc.dim_y == 0 )
            return dsc;
        img_descriptor tmp = dsc;
        tmp.pData = reinterpret_cast<byte*>( dsc.pData ) + (dsc.pitch * (dsc.dim_y - 1));
        tmp.pitch = -dsc.pitch;
        return tmp;
    }
}

#endif // TCAM_IMAGE_TRANSFORM_BASE_H
