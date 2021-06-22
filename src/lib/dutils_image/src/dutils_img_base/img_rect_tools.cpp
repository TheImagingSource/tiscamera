
#include "img_rect_tools.h"
#include "interop_private.h"


img::img_descriptor     img::make_safe_img_view( const img_descriptor& data, const img::rect& r ) noexcept
{
    assert( !img::is_multi_plane_format( data.fourcc_type() ) );
    if( img::is_multi_plane_format( data.fourcc_type() ) ) {
        return data;
    }

    img::rect roi = intersect_with_image_dim( normalize( r ), data.dimensions() );
    if( is_empty_rect( roi ) ) {
        return data;
    }

    uint8_t* ptr = img::get_pixel_start( data, roi.get_top_left() );

    auto dst_type = img::make_img_type( data.type, roi.dimensions() );
    return img::make_img_desc_raw( dst_type, img_plane{ ptr, data.pitch() } );
}

img::rect img::clip_to_img_desc_region( img::rect roi, img::point partial_scan_offset, img::dim pixel_dim, img::dim image_dim ) noexcept
{
    // skip empty rects
    if( roi.is_null() ) {
        return {};
    }
    if( pixel_dim.cx == 0 ) {
        pixel_dim.cx = 1;
    }
    if( pixel_dim.cy == 0 ) {
        pixel_dim.cy = 1;
    }

    roi = normalize( roi );

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
    if( is_full_image( roi, image_dim ) || is_empty_rect( roi ) )
    {
        return {};
    }

    return roi;
}
