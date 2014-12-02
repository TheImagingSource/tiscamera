
#pragma once

#include "../image_transform_base.h"

namespace img
{
    RECT    normalize_and_fit( RECT r, int src_width, int src_height );

    inline bool    is_full_image( const RECT& r, unsigned src_width, unsigned src_height )
    {
        return (r.left == 0 && r.top == 0) && ((r.bottom == 0 && r.right == 0) || ((unsigned)r.bottom == src_height && (unsigned)r.right == src_width));
    }

    inline bool    is_full_image( const RECT& r, const img::img_descriptor& data )
    {
        return is_full_image( r, data.dim_x, data.dim_y );
    }

    img::img_descriptor  make_img_view( const img::img_descriptor& data, const RECT& r );

    inline bool         is_empty_rect( RECT r )
    {
        return r.left == 0 && r.top == 0 && r.right == 0 && r.bottom == 0;
    }

    /** Clips the input RECT to the actual img_desc region.
     * An output of { 0,0,0,0 } is generated when the input roi is empty, when 
     * If the input was { 0,0,0,0 }, the  resulting region 
     * @param roi                   The region of interest in sensor units. 
     * @param partial_scan_offset   The offset from the sensor origin in sensor units. The result will be offset by this.
     * @param pixel_dim             The size of a pixel in the img_desc, so if 4x4 binning is enabled, the sensor position of 20/20 is actually at the img_desc position of 5/5.
     * @param image_dim             The size of the image in img_desc. The resulting roi is clipped to the region.
     */
    RECT    clip_to_img_desc_region( RECT roi, POINT partial_scan_offset, SIZE pixel_dim, SIZE image_dim );
    
    inline 
    RECT    clip_to_img_desc_region( RECT roi, POINT partial_scan_offset, SIZE pixel_dim, const img::img_descriptor& image )
    {
        SIZE tmp = { image.dim_x, image.dim_y };
        return clip_to_img_desc_region( roi, partial_scan_offset, pixel_dim, tmp );
    }

    void    fill_image( const img::img_descriptor& data, unsigned byte_value );
};
