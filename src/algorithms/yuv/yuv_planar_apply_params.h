
#pragma once

#include "../image_transform_base.h"

namespace img
{
namespace yuv
{
    namespace detail {
        // contrast=[-2.0f;+2.0f], brightness=[-1.0f;+1.0f]
        void    calc_y_factors( float& m, float& b, float brightness, float contrast );
        void    calc_uv_factors( float& mul_uv, float& mul_vu, float& add, float saturation, float hue );

        // y8 stuff
        void	apply_uv_params_c_v0( byte* base_ptr, int pixel_count, float saturation, float hue );
        // Y16 stuff
        // void    apply_params_y16_c( img::img_descriptor& dst, float brightness, float contrast );
        // hue[-180;180], saturation[0;256] 64 ^= neutral
        // void    apply_uv_params_y16( img::img_descriptor& dst, float saturation, float hue, unsigned cpu_features );
        void    apply_uv_params_y16_c( img::img_descriptor& dst, float saturation, float hue );
    };

    // pixel = [0;1.0], brightness = [-1.0;1.0], brightness = [-2.0;2.0]
    float   calc_y_pixel_value( float pixel, float brightness, float contrast );

    // actual entry point for libraries

    // supports Y800, Y16, YUV8PLANAR, YUV16PLANAR
    void    apply_y_params( img::img_descriptor& dst, float brightness, float contrast, unsigned cpu_features );

    // contrast=[-512;+512], brightness=[-255;+255]
    inline    void    apply_y_params( img::img_descriptor& dst, int brightness, int contrast, unsigned cpu_features )
    {
        apply_y_params( dst, brightness / 256.0f, contrast / 256.0f, cpu_features );
    }

    // supports Y800, Y16, YUV8PLANAR, YUV16PLANAR
    // hue=[-1.0f;+1.0f], saturation=[0.0f,4.0f]
    void    apply_uv_params( img::img_descriptor& dst, float saturation, float hue, unsigned cpu_features );

};
};
