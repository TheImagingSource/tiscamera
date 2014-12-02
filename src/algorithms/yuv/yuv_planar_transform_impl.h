
#pragma once

#include "../image_transform_base.h"
#include "../img/cpu_features.h"

namespace img
{
namespace yuv
{
    namespace detail {

        void	copy_yuv8planar_uv_planes( byte* dst_ptr, byte* src_ptr, int plane_size );
        void	copy_yuv8planar_uv_planes( byte* dst_ptr, byte* src_ptr, int width, int height );
    }

    void	transform_RGB32_to_YUV8planar( img::img_descriptor& dest, const img::img_descriptor& src, unsigned cpu_features );
    void    transform_YUV8planar_to_dest( img::img_descriptor& dest, const img::img_descriptor& src, unsigned cpu_features );


    void	copy_yuv_planar_uv_planes( img::img_descriptor& dst, const img::img_descriptor& src );

    void	copy_yuv8planar_uv_planes( img::img_descriptor& dst, const img::img_descriptor& src );
    void	copy_yuv16planar_uv_planes( img::img_descriptor& dst, const img::img_descriptor& src );
};
};
