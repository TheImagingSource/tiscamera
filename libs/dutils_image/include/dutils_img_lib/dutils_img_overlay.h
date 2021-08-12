
#ifndef IMG_OVERLAY_H_INC_
#define IMG_OVERLAY_H_INC_

#pragma once

#include <dutils_img/dutils_img.h>
#include <dutils_img/pixel_structs.h>

namespace img_lib {
namespace overlay
{
    using rgba = img::pixel_type::R8G8B8A8;

    namespace color
    {
        constexpr rgba black =          {0x00, 0x00, 0x00, 0xFF};
        constexpr rgba white =          {0xFF, 0xFF, 0xFF, 0xFF};
        constexpr rgba grey =           {0x80, 0x80, 0x80, 0xFF};
        constexpr rgba red =            {0xFF, 0x00, 0x00, 0xFF};
        constexpr rgba green =          {0x00, 0xFF, 0x00, 0xFF};
        constexpr rgba blue =           {0x00, 0x00, 0xFF, 0xFF};
        constexpr rgba yellow =         {0xFF, 0xFF, 0x00, 0xFF};
        constexpr rgba cyan =           {0xFF, 0xFF, 0xFF, 0xFF};
        constexpr rgba magenta =        {0xFF, 0x00, 0xFF, 0xFF};
        constexpr rgba transparent = { 0x00, 0x00, 0x00, 0x00 };
    }

    /**  Helper function to allow several overlay strings to be placed into the video stream of cameras
     * @param img_dim   The dimension of the image to render into
     * @param index     The index of the overlay line to calculate the start position for
     */
    constexpr std::pair<img::point,int>  calc_overlay_start_pos( const img::dim img_dim, const int index ) noexcept
    {
        const int scale = (img_dim.cx / 1024) + 1;

        const int y_offset = 10;
        const int x_offset = 10;

        return { img::point{ x_offset, y_offset + y_offset * scale * index }, scale };
    }

    constexpr int POSITION_CENTER = -1;

    /**
     * Renders a text line to the image descriptor.
     * Note: Only alpha channels of 0x00 and 0xFF are currently supported.
     */
    void	render_text( const img::img_descriptor& dst, img::point pos, int scaling, const char* overlay_text, size_t overlay_text_len, rgba foreground_color, rgba background_color );


    inline void	render_text_in_stream( const img::img_descriptor& dst, int index, const char* overlay_text, size_t overlay_text_len, 
        rgba foreground_color = color::red, rgba background_color = color::white )
    {
        auto [pos, scale] = img_lib::overlay::calc_overlay_start_pos( dst.dimensions(), index );

        img_lib::overlay::render_text( dst, pos, scale, overlay_text, overlay_text_len, foreground_color, background_color );

    }

} // overlay
} // img_lib

#endif // IMG_OVERLAY_H_INC_
