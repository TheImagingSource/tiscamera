
// #include "stdafx.h"

#include "img_desc_tools.h"

#if defined KERNEL_DRIVER_
namespace std {
    template<class T>
    void swap( T& l, T& r )
    {
        T tmp = r;
        r = l;
        l = tmp;
    }
};
#else
#include <utility>
#endif

RECT img::normalize_and_fit( RECT r, int src_width, int src_height )
{
    // fix point order, we always want top left to be top-left and bottom-right ....
    if( r.left > r.right )
        std::swap( r.left, r.right );
    if( r.top > r.bottom )
        std::swap( r.top, r.bottom );

    // now clip to actual dimensions of the image
    r.left = CLIP( r.left, 0, src_width );
    r.top = CLIP( r.top, 0, src_height );
    r.right = CLIP( r.right, 0, src_width );
    r.bottom = CLIP( r.bottom, 0, src_height );

    RECT null_rect = {};

    int res_width = r.right - r.left;
    int res_height = r.bottom - r.top;
    if( res_width == 0 || res_height == 0 )     // if one of the resulting dimensions is 0, we just declare that we use the whole image
        return null_rect;

    if( res_width == src_width || res_height == src_height )        // if the resulting dimensions are equivalent just use the whole image
        return null_rect;

    return r;   // here at least one dimension is changed
}

img::img_descriptor img::make_img_view( const img_descriptor& data, const RECT& r )
{
    ASSERT( !img::is_multi_plane_format( data.type ) );
    if( img::is_multi_plane_format( data.type ) )
        return data;

    RECT roi = normalize_and_fit( r, data.dim_x, data.dim_y );

    if( is_full_image( roi, data.dim_x, data.dim_y ) )
        return data;

    int src_width = (int)data.dim_x;
    int src_height = (int)data.dim_y;

    ASSERT( roi.left >= 0 && roi.left <= roi.right && roi.left < src_width );
    ASSERT( roi.top >= 0 && roi.top <= roi.bottom && roi.top < src_height );
    ASSERT( roi.right >= 0 && roi.right <= src_width );
    ASSERT( roi.bottom >= 0 && roi.bottom <= src_height );

    int res_width = roi.right - roi.left;
    int res_height = roi.bottom - roi.top;

    int bpp = img::get_bits_per_pixel( data.type );
    byte* ptr = data.pData + roi.top * data.pitch + (roi.left * bpp) / 8;

    return img::to_img_desc( ptr, data.type, res_width, res_height, data.pitch, res_height * data.pitch );
}

void img::fill_image( const img_descriptor& data, unsigned byte_value )
{
    int bytes_to_fill_per_line = (img::get_bits_per_pixel( data.type ) * data.dim_x) / 8;
    for( unsigned y = 0; y < data.dim_y; ++y )
    {
        byte* line = data.pData + y * data.pitch;
        memset( line, byte_value, bytes_to_fill_per_line );
    }
}

RECT img::clip_to_img_desc_region( RECT roi, POINT partial_scan_offset, SIZE pixel_dim, SIZE image_dim )
{
    // skip empty rects
    if( is_empty_rect( roi ) )
        return roi;
    if( roi.left > roi.right )
        std::swap( roi.left, roi.right );
    if( roi.top > roi.bottom )
        std::swap( roi.top, roi.bottom );

    // offset by image partial scan offset which designates the position relative to the sensor origin
    roi.top -= partial_scan_offset.y;
    roi.left -= partial_scan_offset.x;
    roi.bottom -= partial_scan_offset.y;
    roi.right -= partial_scan_offset.x;

    // scale the rect down to the specific dimensions to compensate for binning/skipping
    roi.top /= pixel_dim.cx;
    roi.left /= pixel_dim.cx;
    roi.bottom /= pixel_dim.cy;
    roi.right /= pixel_dim.cy;

    // clip the resulting rect to contain only the portion that is visible in the actual image
    roi.left = CLIP( roi.left, 0, image_dim.cx );
    roi.top = CLIP( roi.top, 0, image_dim.cy );
    roi.right = CLIP( roi.right, 0, image_dim.cx );
    roi.bottom = CLIP( roi.bottom, 0, image_dim.cy );

    // now recheck the empty/full image conditions and return a { 0, 0, 0, 0 } to indicate the full image
    if( ((roi.left == 0 && roi.bottom == 0) && ((roi.top == 0 && roi.bottom == 0) || (roi.right == image_dim.cx && roi.bottom == image_dim.cy)))
        || (roi.right - roi.left == 0) || (roi.bottom - roi.top == 0) )
    {
        RECT r = {};
        return r;
    }

    return roi;
}
