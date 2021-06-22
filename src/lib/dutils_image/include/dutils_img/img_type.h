
#pragma once

#include "image_helper_types.h"
#include "image_fourcc.h"
#include "image_fourcc_func.h"

#include <cstdint>
#include <cassert>

namespace img
{
    struct img_type
    {
        uint32_t		type = 0;           // FOURCC of this image type

        img::dim        dim;                //> Dimensions in pixel for this image type

        int	            buffer_length = 0;  //> Minimum buffer length required by type and dim for a full image

        constexpr bool      empty() const noexcept { return type == 0 || dim.empty(); }
        constexpr fourcc    fourcc_type() const noexcept { return static_cast<fourcc>(type); }
    };

    /** Test if both types are the same, ignoring buffer length, so 'lhs.dim == rhs.dim && lhs.type == rhs.type'
     */
    constexpr bool    is_same_type( const img::img_type& lhs, const img::img_type& rhs ) noexcept;
    constexpr bool    operator==( const img::img_type& lhs, const img::img_type& rhs ) noexcept;
    constexpr bool    operator!=( const img::img_type& lhs, const img::img_type& rhs ) noexcept;

    constexpr img_type          make_img_type( uint32_t fcc, img::dim dim, int image_size ) noexcept;
    constexpr img_type          make_img_type( uint32_t fcc, img::dim dim ) noexcept;
    constexpr img_type          make_img_type_with_bpp( uint32_t fcc, img::dim dim, int bpp ) noexcept;

    constexpr img_type          make_img_type( fourcc fcc, img::dim dim, int image_size ) noexcept;
    constexpr img_type          make_img_type( fourcc fcc, img::dim dim ) noexcept;
    constexpr img_type          make_img_type_with_bpp( fourcc fcc, img::dim dim, int bpp ) noexcept;

    /* Test if type_to_test is the same as orig_type or at least fits into the orig_type.
     * @param orig_type     Original type, to which to test against.
     * @param type_to_test  Image type to test
     */
    constexpr bool              is_compatible( const img::img_type& orig_type, const img::img_type& type_to_test ) noexcept;
    constexpr int               calc_minimum_pitch( img::img_type type ) noexcept;
    constexpr int               calc_minimum_buffer_length( img::img_type type ) noexcept;

    namespace planar
    {
        constexpr int     get_plane_size( img::img_type type, int plane_index ) noexcept { return get_plane_size( type.fourcc_type(), type.dim, plane_index ); }
        constexpr int     get_plane_pitch_minimum( img::img_type type, int plane_index ) noexcept { return get_plane_pitch_minimum( type.fourcc_type(), type.dim.cx, plane_index ); }
    }

    //////////////////////////////////////////////////////////////////////////
    /// Implementations:

    constexpr bool    is_same_type( const img::img_type& lhs, const img::img_type& rhs ) noexcept
    {
        return lhs.type == rhs.type && lhs.dim == rhs.dim;
    }
    constexpr bool    operator==( const img::img_type& lhs, const img::img_type& rhs ) noexcept
    {
        return is_same_type( lhs, rhs ) && lhs.buffer_length == rhs.buffer_length;
    }
    constexpr bool    operator!=( const img::img_type& lhs, const img::img_type& rhs ) noexcept
    {
        return !(lhs == rhs);
    }

    constexpr int               calc_minimum_pitch( img::img_type type ) noexcept
    {
        return calc_minimum_pitch( type.fourcc_type(), type.dim.cx );
    }
    constexpr int               calc_minimum_buffer_length( img::img_type type ) noexcept
    {
        return calc_minimum_img_size( type.fourcc_type(), type.dim );
    }
    constexpr img_type         make_img_type( uint32_t fcc, img::dim dim, int image_size ) noexcept { return img::img_type{ fcc, dim, image_size }; }
    constexpr img_type         make_img_type( uint32_t fcc, img::dim dim ) noexcept
    {
        assert( img::get_bits_per_pixel( fcc ) != 0 );
        return make_img_type( fcc, dim, calc_minimum_img_size( (img::fourcc) fcc, dim ) );
    }
    constexpr img_type         make_img_type_with_bpp( uint32_t fcc, img::dim dim, int bpp ) noexcept
    {
        if( img::get_bits_per_pixel( fcc ) == 0 )
        {
            return make_img_type( fcc, dim, calc_img_size_from_bpp( dim, bpp ) );
        }
        return make_img_type( fcc, dim, calc_minimum_img_size( (img::fourcc) fcc, dim ) );
    }

    constexpr img_type         make_img_type( fourcc fcc, img::dim dim, int image_size ) noexcept { return img::img_type{ static_cast<uint32_t>(fcc), dim, image_size }; }
    constexpr img_type         make_img_type( fourcc fcc, img::dim dim ) noexcept { return make_img_type( static_cast<uint32_t>(fcc), dim ); }
    constexpr img_type         make_img_type_with_bpp( fourcc fcc, img::dim dim, int bpp ) noexcept { return make_img_type_with_bpp( static_cast<uint32_t>(fcc), dim, bpp ); }

    /* Test if type_to_test is the same as orig_type or at least fits into the orig_type.
     * @param orig_type     Original type, to which to test against.
     * @param type_to_test  Image type to test
     */
    constexpr bool      is_compatible( const img::img_type& orig_type, const img::img_type& type_to_test ) noexcept
    {
        if( type_to_test.type != orig_type.type ) {
            return false;
        }
        if( type_to_test.dim.cx > orig_type.dim.cx || type_to_test.dim.cy > orig_type.dim.cy ) {
            return false;
        }
        //if( type_to_test.buffer_length < orig_type.buffer_length ) {
        //	return false;
        //}
        return true;
    }

}