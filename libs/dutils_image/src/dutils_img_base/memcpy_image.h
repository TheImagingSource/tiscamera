
#ifndef MEMCPY_IMAGE_H_INC_
#define MEMCPY_IMAGE_H_INC_

#pragma once

#include <cstdint>
#include <dutils_img/image_transform_base.h>

namespace img
{
    struct img_descriptor;

    void	memcpy_image( const img::img_descriptor& dst, const img::img_descriptor& src ) noexcept;
    void	memcpy_image( img::img_plane dst, img::img_plane src, int dim_y, int byter_per_line ) noexcept;
    void	memcpy_image( void* dst_ptr, int dst_pitch, void* src_ptr, int src_pitch, int bytes_per_line, int dim_y, bool bFlip ) noexcept;

    void    fill_image( const img::img_descriptor& data, uint8_t byte_value ) noexcept;
}

#endif // MEMCPY_IMAGE_H_INC_
