
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
    struct img_plane {
        void*   plane_ptr = nullptr;
        int     pitch = 0;
    };

    struct img_planar_layout_data {
        img_plane   planes[4];
    };

    struct img_descriptor
    {
        uint32_t		type = 0;           // FOURCC of this image

        img::dim        dim;

        int	            data_length = 0;    // the actual data length valid in this buffer. this is in most cases calc_img_size( type, dim_x, dim_y ), but may be smaller for e.g. compressed formats 

        uint32_t        flags = 0;          // flags, these should only used in the library itself

        img_planar_layout_data       data_;

        enum {
            flags_no_wrap_beg = 0x1,    // it is not necessary to wrap on the start line (e.g. debayering may access p_data + pitch * -1 and p_data + pitch * -2)
            flags_no_wrap_end = 0x2,    // it is not necessary to wrap on the end line (e.g. debayering may access p_data + data_length + pitch and p_data + data_length + pitch
            flags_no_flip = 0x4,        // this indicates that the image is already flipped, this flag is needed when separating the image in strips
            flags_read_only = 0x8,
        };

        constexpr fourcc        fourcc_type() const noexcept { return static_cast<fourcc>(type); }

        constexpr uint8_t*      data() const noexcept { return static_cast<uint8_t*>(data_.planes[0].plane_ptr ); }

        /** Returns the image stride in bytes. Note: This only works for the first plane */
        constexpr int	        pitch() const noexcept { return data_.planes[0].pitch; }
        constexpr size_t        size() const noexcept { return static_cast<size_t>(data_length); }

        constexpr img::dim          dimensions() const noexcept { return dim; }
        constexpr bool              empty() const noexcept { return dim.empty() || pitch() == 0 || type == 0 || data() == nullptr; }
        constexpr img::img_type     to_img_type() const noexcept { return make_img_type( type, dim ); }
        constexpr img::img_plane    plane( int index ) const noexcept { return data_.planes[index]; }
    };

    /** Creates a img_descriptor from raw structures and does not calculate planes or other things */
    constexpr img_descriptor	make_img_desc_raw( img::img_type type, img_plane plane0 ) noexcept;
    constexpr img_descriptor	make_img_desc_raw( img::img_type type, img_planar_layout_data plane0 ) noexcept;
    constexpr img_descriptor	make_img_desc_raw( fourcc fcc, img::dim dim, int data_length, img_plane plane0, uint32_t flags = 0 ) noexcept;
    constexpr img_descriptor	make_img_desc_raw( fourcc fcc, img::dim dim, int data_length, img_planar_layout_data plane0, uint32_t flags = 0 ) noexcept;
    constexpr img_descriptor	make_img_desc_from_linear_memory( img::img_type in_type, void* data_ptr ) noexcept;

    constexpr img_descriptor	replace_fcc( const img::img_descriptor& desc, img::fourcc new_fcc ) noexcept;
    
    constexpr img_descriptor    flip_image_in_img_desc( const img_descriptor& dsc ) noexcept;
    constexpr img_descriptor    flip_image_in_img_desc_if_allowed( const img_descriptor& src ) noexcept;

    constexpr uint8_t*          get_line_start( const img::img_descriptor& dsc, int y ) noexcept;
    constexpr uint8_t*          get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept;
    constexpr uint8_t*          get_line_start( const img::img_plane& dsc, int y ) noexcept;

    template<class TOut>
    constexpr TOut*             get_line_start( const img::img_descriptor& dsc, int y ) noexcept;
    template<class TOut>
    constexpr TOut*             get_line_start( const img::img_plane& dsc, int y ) noexcept;
    template<class TOut>
    constexpr TOut*             get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept;

    img_descriptor    extract_plane( const img::img_descriptor& dsc, int plane_index ) noexcept;
    img_descriptor    extract_plane( const img::img_descriptor& dsc, int plane_index, fourcc fcc ) noexcept;

    constexpr bool is_fcc_in_fcclist( const img::img_type& type, std::initializer_list<fourcc> typelist ) noexcept;
    constexpr bool is_fcc_in_fcclist( const img::img_descriptor& type, std::initializer_list<fourcc> typelist ) noexcept;


    constexpr img_descriptor	make_img_desc_from_linear_memory( img::img_type in_type, void* data_ptr ) noexcept
    {
        if( img::is_multi_plane_format( in_type.fourcc_type() ) )
        {
            img_planar_layout_data plane_data;
            auto* data_ptr_iter = static_cast<uint8_t*>( data_ptr );
            for( int plane_index = 0; plane_index < img::planar::get_plane_count( in_type.fourcc_type() ); ++plane_index )
            {
                const int plane_pitch = img::planar::get_plane_pitch_minimum( in_type, plane_index );
                plane_data.planes[plane_index] = img_plane{ data_ptr_iter, plane_pitch };

                const int plane_size = img::planar::get_plane_size( in_type, plane_index );
                data_ptr_iter += plane_size;
            }
            return make_img_desc_raw( in_type.fourcc_type(), in_type.dim, in_type.buffer_length, plane_data );
        }
        const int pitch = img::calc_minimum_pitch( in_type );
        return make_img_desc_raw( in_type.fourcc_type(), in_type.dim, in_type.buffer_length, img_plane{ data_ptr, pitch } );
    }

    constexpr img_descriptor	make_img_desc_raw( img_type type, img_plane plane0 ) noexcept
    {
        return make_img_desc_raw( type.fourcc_type(), type.dim, type.buffer_length, plane0 );
    }

    constexpr img_descriptor	make_img_desc_raw(img::img_type type, img_planar_layout_data plane0) noexcept
    {
      return make_img_desc_raw(type.fourcc_type(), type.dim, type.buffer_length, plane0);
    }

    constexpr img_descriptor	make_img_desc_raw( fourcc fcc, img::dim dim, int data_length, img_plane plane0, uint32_t flags /*= 0*/ ) noexcept
    {
        img_descriptor res{
            static_cast<uint32_t>( fcc ),
            dim,
            data_length,
            flags,
            { plane0 },
        };
        return res;
    }
    constexpr img_descriptor	make_img_desc_raw( fourcc fcc, img::dim dim, int data_length, img_planar_layout_data layout, uint32_t flags /*= 0*/ ) noexcept
    {
        return img_descriptor{
            static_cast<uint32_t>(fcc),
            dim,
            data_length,
            flags,
            layout,
        };
    }

    constexpr img_descriptor	replace_fcc( const img::img_descriptor& dsc, img::fourcc new_fcc ) noexcept
    {
        auto res = dsc;
        res.type = (uint32_t) new_fcc;
        return res;
    }

    constexpr img_plane  flip_img_plane( const img_descriptor& dsc, int plane_index ) noexcept
    {
        const auto plane = dsc.data_.planes[plane_index];
        if( dsc.dim.cy == 0 ) {
            return plane;
        }
        return img::img_plane{
            static_cast<uint8_t*>(plane.plane_ptr) + plane.pitch * (dsc.dim.cy - 1),
            -plane.pitch,
        };
    }

    constexpr img_descriptor  flip_image_in_img_desc( const img_descriptor& dsc ) noexcept
    {
        img_descriptor tmp = dsc;
        for( int index = 0; index < img::calc_planar_fmt_count_of_planes( dsc.fourcc_type() ); ++index ) {
            tmp.data_.planes[index] = flip_img_plane( dsc, index );
        }
        return tmp;
    }

    constexpr img_descriptor flip_image_in_img_desc_if_allowed( const img_descriptor& src ) noexcept
    {
        if( !(src.flags & img::img_descriptor::flags_no_flip) ) {
            return img::flip_image_in_img_desc( src );
        }
        return src;
    }

    constexpr uint8_t* get_line_start( void* ptr, int pitch, int y ) noexcept {
        return static_cast<uint8_t*>( ptr ) + static_cast<int64_t>( y ) * pitch;
    }
    constexpr uint8_t* get_line_start( const img::img_descriptor& dsc, int y ) noexcept
    {
        return dsc.data() + y * dsc.pitch();
    }
    constexpr uint8_t* get_pixel_start( const img::img_descriptor& dsc, img::point pos ) noexcept
    {
        int bpp = img::get_bits_per_pixel( dsc.type );
        assert( bpp != 0 );
        return dsc.data() + pos.y * dsc.pitch() + (pos.x * bpp) / 8;
    }

    template<class TOut>
    constexpr TOut* get_line_start( const img::img_descriptor& dsc, int y ) noexcept
    {
        return reinterpret_cast<TOut*>(dsc.data() + y * dsc.pitch());
    }

    template<class TOut>
    constexpr TOut* get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept
    {
        return reinterpret_cast<TOut*>(get_line_start_of_plane( dsc, y, plane_index ) );
    }
    constexpr uint8_t* get_line_start_of_plane( const img::img_descriptor& dsc, int y, int plane_index ) noexcept
    {
        return static_cast<uint8_t*>(dsc.data_.planes[plane_index].plane_ptr) + y * dsc.data_.planes[plane_index].pitch;
    }

    constexpr uint8_t* get_line_start( const img::img_plane& dsc, int y ) noexcept
    {
        return static_cast<uint8_t*>( dsc.plane_ptr ) + y * dsc.pitch;
    }
    template<class TOut>
    constexpr TOut* get_line_start( const img::img_plane& dsc, int y ) noexcept
    {
        return reinterpret_cast<TOut*>(static_cast<uint8_t*>(dsc.plane_ptr) + y * dsc.pitch);
    }

    inline img::img_descriptor extract_plane( const img::img_descriptor& dsc, int plane_index, fourcc fcc ) noexcept
    {
        const auto plane = dsc.data_.planes[plane_index];

        const auto info = planar::get_fcc_info( dsc.fourcc_type(), plane_index );
        const img::dim actual_dim{ static_cast<int>(dsc.dim.cx * info.scale_dim_x), static_cast<int>(dsc.dim.cy * info.scale_dim_y) };

        const auto plane_size = plane.pitch * actual_dim.cy;
        return make_img_desc_raw( fcc, actual_dim, plane_size, plane );
    }

    inline img::img_descriptor extract_plane( const img::img_descriptor& dsc, int plane_index ) noexcept
    {
        const auto info = planar::get_fcc_info( dsc.fourcc_type(), plane_index );
        const img::dim actual_dim{ static_cast<int>(dsc.dim.cx * info.scale_dim_x), static_cast<int>(dsc.dim.cy * info.scale_dim_y) };

        const auto plane = dsc.data_.planes[plane_index];
        const auto plane_size = plane.pitch * actual_dim.cy;
        return make_img_desc_raw( info.equiv_fcc, actual_dim, plane_size, plane );
    }

    constexpr bool is_fcc_in_fcclist( const img::img_type& type, std::initializer_list<fourcc> typelist ) noexcept
    {
        return is_fcc_in_fcclist( type.fourcc_type(), typelist );
    }
    constexpr bool is_fcc_in_fcclist( const img::img_descriptor& type, std::initializer_list<fourcc> typelist ) noexcept
    {
        return is_fcc_in_fcclist( type.fourcc_type(), typelist );
    }
    constexpr bool is_fcc_in_fcclist( const img::img_type& type, img::fourcc fcc ) noexcept
    {
        return type.fourcc_type() == fcc;
    }
    constexpr bool is_fcc_in_fcclist( const img::img_descriptor& type, img::fourcc fcc ) noexcept
    {
        return type.fourcc_type() == fcc;
    }
}

#endif // IMAGE_TRANSFORM_BASE_H_INC_