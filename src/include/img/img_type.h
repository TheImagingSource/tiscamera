
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
        uint32_t		type;               // FOURCC of this image type

        img::dim        dim;                // dimensions in pixel for this image type

        int	            buffer_length;      // this is the minimum length of the buffer to be allocated for a full image

        constexpr bool  empty() const noexcept { return type == 0 || dim.empty(); }
    };

    /** Test if both types are the same, ignoring buffer length, so 'lhs.dim == rhs.dim && lhs.type == rhs.type'
 */
    constexpr bool    is_same_type( const img::img_type& lhs, const img::img_type& rhs ) noexcept { return lhs.type == rhs.type && lhs.dim == rhs.dim; }
    constexpr bool    operator==( const img::img_type& lhs, const img::img_type& rhs ) noexcept { return is_same_type( lhs, rhs ) && lhs.buffer_length == rhs.buffer_length; }
    constexpr bool    operator!=( const img::img_type& lhs, const img::img_type& rhs ) noexcept { return !(lhs == rhs); }

    constexpr img_type          make_img_type( uint32_t fcc, img::dim dim, int image_size ) noexcept;
    constexpr img_type          make_img_type( uint32_t fcc, img::dim dim ) noexcept;
    constexpr img_type          make_img_type_with_bpp( uint32_t fcc, img::dim dim, int bpp ) noexcept;
    constexpr bool              is_compatible( const img::img_type& orig_type, const img::img_type& type_to_test ) noexcept;

    constexpr int               calc_minimum_pitch( img::img_type type ) noexcept
    {
        return calc_minimum_pitch( type.type, type.dim.cx );
    }
    constexpr img_type         make_img_type( uint32_t fcc, img::dim dim, int image_size ) noexcept
    {
        return img::img_type{ fcc, dim.cx, dim.cy, image_size };
    }
    constexpr img_type         make_img_type( uint32_t fcc, img::dim dim ) noexcept
    {
        assert( img::get_bits_per_pixel( fcc ) != 0 );
        return make_img_type( fcc, dim, calc_minimum_img_size( fcc, dim ) );
    }
    constexpr img_type         make_img_type_with_bpp( uint32_t fcc, img::dim dim, int bpp ) noexcept
    {
        if( img::get_bits_per_pixel( fcc ) == 0 )
        {
            return make_img_type( fcc, dim, calc_img_size_from_bpp( dim, bpp ) );
        }
        return make_img_type( fcc, dim, calc_minimum_img_size( fcc, dim ) );
    }

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