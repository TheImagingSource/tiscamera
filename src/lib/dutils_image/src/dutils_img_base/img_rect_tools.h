
#pragma once

#include <dutils_img/dutils_img.h>

namespace img
{
    constexpr bool          is_full_image( const img::rect& r, img::dim image_dim ) noexcept;
    constexpr bool          is_full_image( const img::rect& r, const img::img_descriptor& data ) noexcept;
    
    /** 
     * Creates a 'view' on the image in data using the rect in r.
     * If r is outside of the range of data, this functions just returns data.
     */
    img::img_descriptor     make_safe_img_view( const img::img_descriptor& data, const img::rect& r ) noexcept;


    /** Clips the input RECT to the actual img_desc region.
     * An output of { 0,0,0,0 } is generated when the input roi is empty, when 
     * If the input was { 0,0,0,0 }, the  resulting region 
     * @param roi                   The region of interest in sensor units. 
     * @param partial_scan_offset   The offset from the sensor origin in sensor units. The result will be offset by this.
     * @param pixel_dim             The size of a pixel in the img_desc, so if 4x4 binning is enabled, the sensor position of 20/20 is actually at the img_desc position of 5/5.
     * @param image_dim             The size of the image in img_desc. The resulting roi is clipped to the region.
     */
    img::rect    clip_to_img_desc_region( img::rect roi, img::point partial_scan_offset, img::dim pixel_dim, img::dim image_dim ) noexcept;
    img::rect    clip_to_img_desc_region( img::rect roi, img::point partial_scan_offset, img::dim pixel_dim, const img::img_descriptor& image ) noexcept;
    
    inline 
    img::rect    clip_to_img_desc_region( img::rect roi, img::point partial_scan_offset, img::dim pixel_dim, const img::img_descriptor& image ) noexcept
    {
        return clip_to_img_desc_region( roi, partial_scan_offset, pixel_dim, image.dimensions() );
    }

    constexpr bool    is_full_image( const img::rect& r, const img::dim image_dim ) noexcept
    {
        return (r.left == 0 && r.top == 0) && ((r.bottom == 0 && r.right == 0) || (r.bottom == image_dim.cy && r.right == image_dim.cx));
    }
    constexpr bool    is_full_image( const img::rect& r, const img::img_descriptor& data ) noexcept
    {
        return is_full_image( r, data.dimensions() );
    }
}
