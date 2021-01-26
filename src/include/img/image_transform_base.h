
#ifndef IMAGE_TRANSFORM_BASE_H_INC_
#define IMAGE_TRANSFORM_BASE_H_INC_

#pragma once

#include "image_helper_types.h"
#include "image_fourcc.h"
#include "image_fourcc_func.h"

#include <cstdint>
#include <cassert>

#include "img_type.h"


namespace img
{
    struct img_descriptor
    {
        uint32_t		type;           // FOURCC of this image

        img::dim        dim;


        int	            data_length;    // the actual data length valid in this buffer. this is in most cases calc_img_size( type, dim_x, dim_y ), but may be smaller for e.g. compressed formats 

        int	            pitch;	        // in bytes, this is the count of bytes it takes from the start of one line to get to the next line
                                        // for planar formats this contains the count of bytes to get from one line in one plane to the next line in the same 
                                        // note that this can be negative

        uint8_t*	    data_ptr;       // this is the pointer to the 'first' byte of the image. this can be the first byte of the last line if pitch is negative

        

        int             plane_pitch;    // == 0 for packed formats, otherwise the count of bytes to skip from one pixel to the according pixel in the next plane
                                        // e.g.: uint8_t* src = data_ptr + plane_pitch; // => src now points to the first pixel of the second plane

        enum {
            flags_no_wrap_beg = 0x1,    // it is not necessary to wrap on the start line (e.g. debayering may access p_data + pitch * -1 and p_data + pitch * -2)
            flags_no_wrap_end = 0x2,    // it is not necessary to wrap on the end line (e.g. debayering may access p_data + data_length + pitch and p_data + data_length + pitch
            flags_no_flip = 0x4,        // this indicates that the image is already flipped, this flag is needed when separating the image in strips
        };

        uint32_t        flags;          // flags, these should only used in the library itself


        constexpr uint8_t*  data() const noexcept { return static_cast<uint8_t*>(data_ptr); }
        constexpr size_t    size() const noexcept { return static_cast<size_t>( data_length ); }

        constexpr img::dim  dimensions() const noexcept { return dim; }
        constexpr bool      empty() const noexcept { return dim.empty() || pitch == 0 || type == 0 || data_ptr == nullptr; }
        constexpr img::img_type to_img_type() const noexcept { return make_img_type( type, dim ); }
    };

    constexpr img_descriptor	make_img_desc( void* data, uint32_t fcc, img::dim dim, int line_pitch, int size, int plane_pitch, uint32_t flags ) noexcept;
    constexpr img_descriptor	make_img_desc( void* data, uint32_t fcc, img::dim dim, int pitch, int size ) noexcept;
    constexpr img_descriptor	make_img_desc( void* data, uint32_t fcc, img::dim dim ) noexcept;

    constexpr img_descriptor	to_img_desc( const img_type& type, void* data ) noexcept;

    constexpr img_descriptor	copy_img_desc( const img::img_descriptor& desc, void* data ) noexcept;
    constexpr img_descriptor	copy_img_desc( const img::img_descriptor& desc, uint32_t new_fcc ) noexcept;
    
    constexpr img_descriptor    flip_image_in_img_desc( const img_descriptor& dsc ) noexcept;
    constexpr img_descriptor    flip_image_in_img_desc_if_allowed( const img_descriptor& src ) noexcept;

    constexpr uint8_t*          get_line_start( const img::img_descriptor& dsc, int y ) noexcept;
    constexpr uint8_t*          get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept;

    template<class TOut>
    constexpr TOut*             get_line_start( const img::img_descriptor& dsc, int y ) noexcept;
    template<class TOut>
    constexpr TOut*             get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept;

    constexpr img_descriptor	make_img_desc( void* data, uint32_t fcc, img::dim dim, int line_pitch, int size, int plane_pitch, uint32_t flags ) noexcept
    {
        img_descriptor rval = {
            fcc,
            dim,
            size,
            line_pitch,
            static_cast<uint8_t*>(data),
            plane_pitch,
            flags
        };
        return rval;
    }

    constexpr img_descriptor	make_img_desc( void* data, uint32_t fcc, img::dim dim, int pitch, int size ) noexcept
    {
        return make_img_desc( data, fcc, dim, pitch, size, calc_plane_pitch( fcc, dim.cy, pitch ), 0 /* flags */ );
    }
    constexpr img_descriptor	make_img_desc( void* data, uint32_t fcc, img::dim dim ) noexcept
    {
        const int pitch = calc_minimum_pitch( fcc, dim.cx );
        const int size = calc_minimum_img_size( fcc, dim );
        return make_img_desc( data, fcc, dim, pitch, size );
    }

    constexpr img_descriptor	to_img_desc( const img_type& t, void* data, int pitch, int size ) noexcept
    {
        return make_img_desc( data, t.type, t.dim, pitch, size );
    }
    constexpr img_descriptor	to_img_desc( const img_type& t, void* data ) noexcept
    {
        return make_img_desc( data, t.type, t.dim, calc_minimum_pitch( t ), t.buffer_length );
    }
    constexpr img_descriptor	copy_img_desc( const img::img_descriptor& dsc, void* data ) noexcept
    {
        auto tmp = dsc;
        tmp.data_ptr = static_cast<uint8_t*>( data );
        return tmp;
    }
    constexpr img_descriptor	copy_img_desc( const img::img_descriptor& dsc, uint32_t new_fcc ) noexcept
    {
        return make_img_desc( dsc.data(), new_fcc, dsc.dim, dsc.pitch, dsc.data_length );
    }

    constexpr img_descriptor  flip_image_in_img_desc( const img_descriptor& dsc ) noexcept
    {
        img_descriptor tmp = dsc;
        if( dsc.dim.cy != 0 ) {
            tmp.data_ptr = dsc.data() + dsc.pitch * (dsc.dim.cy - 1);
        }
        tmp.pitch = -dsc.pitch;
        return tmp;
    }

    constexpr img_descriptor flip_image_in_img_desc_if_allowed( const img_descriptor& src ) noexcept
    {
        if( !(src.flags & img::img_descriptor::flags_no_flip) ) {
            return img::flip_image_in_img_desc( src );
        }
        return src;
    }

    constexpr uint8_t* get_line_start( void* ptr, int pitch, int y ) noexcept
    {
        return static_cast<uint8_t*>( ptr ) + static_cast<int64_t>( y ) * pitch;
    }
    constexpr uint8_t* get_line_start( const img::img_descriptor& dsc, int y ) noexcept
    {
        return dsc.data() + y * dsc.pitch;
    }
    constexpr uint8_t* get_pixel_start( const img::img_descriptor& dsc, img::point pos ) noexcept
    {
        int bpp = img::get_bits_per_pixel( dsc.type );
        assert( bpp != 0 );
        return dsc.data() + pos.y * dsc.pitch + (pos.x * bpp) / 8;
    }

    template<class TOut>
    constexpr TOut* get_line_start( const img::img_descriptor& dsc, int y ) noexcept
    {
        return reinterpret_cast<TOut*>(dsc.data() + y * dsc.pitch);
    }

    template<class TOut>
    constexpr TOut* get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept
    {
        return reinterpret_cast<TOut*>(dsc.data() + y * dsc.pitch + dsc.plane_pitch * plane_index);
    }
    constexpr uint8_t* get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept
    {
        return dsc.data() + y * dsc.pitch + dsc.plane_pitch * plane_index;
    }
}

#endif // IMAGE_TRANSFORM_BASE_H_INC_