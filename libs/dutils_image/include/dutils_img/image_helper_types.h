
#pragma once

// these must be replaced in a driver context
#include <utility>  // swap
#include <cstdlib>  // abs

namespace img
{
    struct point 
    {
        int x = 0;
        int y = 0;
    };

    constexpr bool operator==( const point& lhs, const point& rhs ) noexcept { return (lhs.x == rhs.x) && (lhs.y == rhs.y); }
    constexpr bool operator!=( const point& lhs, const point& rhs ) noexcept { return !(lhs == rhs); }

    struct dim
    {
        int cx = 0;
        int cy = 0;

        /** Returns if cy == 0 && cy == 0. */
        constexpr bool is_null() const noexcept { return cx == 0 && cy == 0; }
        /** Returns if the dimension does not span a region, cx == 0 || cy == 0 */
        constexpr bool empty() const noexcept { return cx == 0 || cy == 0; }

        constexpr dim& operator*=( int val ) noexcept { cx *= val; cy *= val; return *this; }
        constexpr dim& operator/=( int val ) noexcept { cx /= val; cy /= val; return *this; }

        constexpr dim& operator*=( dim val ) noexcept { cx *= val.cx; cy *= val.cy; return *this; }
        constexpr dim& operator/=( dim val ) noexcept { cx /= val.cx; cy /= val.cy; return *this; }
        constexpr dim& operator+=( dim val ) noexcept { cx += val.cx; cy += val.cy; return *this; }
        constexpr dim& operator-=( dim val ) noexcept { cx -= val.cx; cy -= val.cy; return *this; }
    };

    constexpr bool operator==( const dim& lhs, const dim& rhs ) noexcept { return (lhs.cx == rhs.cx) && (lhs.cy == rhs.cy); }
    constexpr bool operator!=( const dim& lhs, const dim& rhs ) noexcept { return !(lhs == rhs); }

    constexpr dim   operator*( dim d0, dim d1 ) noexcept { return d0 *= d1; }
    constexpr dim   operator/( dim d0, dim d1 ) noexcept { return d0 /= d1; }
    constexpr dim   operator+( dim d0, dim d1 ) noexcept { return d0 += d1; }
    constexpr dim   operator-( dim d0, dim d1 ) noexcept { return d0 -= d1; }

    constexpr dim   operator*( dim d, int val ) noexcept { return d *= val; }
    constexpr dim   operator/( dim d, int val ) noexcept { return d /= val; }
    constexpr dim   operator*( int val, dim d ) noexcept { return d *= val; }


    struct rect
    {
        constexpr rect() noexcept = default;
        constexpr rect( int l, int t, int r, int b ) noexcept : left( l ), top( t ), right( r ), bottom( b ) {}
        constexpr rect( point pos, dim size ) noexcept : left( pos.x ), top( pos.y ), right( pos.x + size.cx ), bottom( pos.y + size.cy ) {}
        constexpr rect( dim size ) noexcept : left( 0 ), top( 0 ), right( size.cx ), bottom( size.cy ) {}

        int     left = 0;
        int     top = 0;
        int     right = 0;
        int     bottom = 0;

        // All values are 0
        constexpr bool          is_null() const noexcept { return left == 0 && top == 0 && right == 0 && bottom == 0; }

        img::dim                dimensions() const noexcept { return img::dim{ abs( right - left ), abs( bottom - top ) }; }
        
        constexpr img::point    get_top_left() const noexcept { return img::point{ left, top }; }
        constexpr img::point    get_bottom_right() const noexcept { return img::point{ right, bottom }; }
    };

    constexpr rect      normalize( rect r ) noexcept
    {
        if( r.left > r.right ) {
            std::swap( r.left, r.right );
        }
        if( r.top > r.bottom) {
            std::swap( r.bottom, r.top );
        }
        return r;
    }

    constexpr bool      is_normalized( rect r ) noexcept { return (r.left <= r.right) && (r.top <= r.bottom); }


    // if dimensions().cx == 0 || dimensions().cy == 0
    // precondition: is_normalized( *this ) == true
    constexpr bool      is_empty_rect( rect r ) noexcept { return (r.right - r.left) == 0 || (r.bottom - r.top) == 0; }


    /**
     * Tests if this region intersects the image region of [{0,0},image_dim].
     * Precondition: is_normalized( region ) == true
     */
    constexpr bool      is_intersect_with_image_dim( img::rect region, img::dim image_dim ) noexcept
    {
        return !(region.bottom < 0 || region.top > image_dim.cy || region.right < 0 || region.left > image_dim.cx);
    }

    /**
     * Creates am intersected rect which is completely in the region spanned by [{0,0},image_dim] and tmp_region.
     * If the rect is outside of the image region, a null rect is returned.
     */
    constexpr   img::rect   intersect_with_image_dim( const img::rect tmp_region, img::dim image_dim ) noexcept
    {
        const auto region = normalize( tmp_region );

        // reject rects outside our image region [0,0,src_dim.cx,src_dim.cy]
        if( !is_intersect_with_image_dim( region, image_dim ) )
        {
            return img::rect{};
        }

        // now clip to actual dimensions of the image
        img::rect rval;
        rval.left = region.left < 0 ? 0 : region.left;
        rval.top = region.top < 0 ? 0 : region.top;
        rval.right = region.right > image_dim.cx ? image_dim.cx : region.right;
        rval.bottom = region.bottom > image_dim.cy ? image_dim.cy : region.bottom;
        return rval;
    }
}



