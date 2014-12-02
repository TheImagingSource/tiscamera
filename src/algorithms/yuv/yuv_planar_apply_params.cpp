// test_yuv.cpp : Defines the entry point for the console application.
//

#include "yuv_planar_apply_params.h"

#include "../sse_helper/sse_utils.h"

#include <emmintrin.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include "../img/cpu_features.h"

#pragma warning ( disable : 4127 )	// warning C4127: conditional expression is constant

using namespace sse_utils;

// see http://msdn.microsoft.com/en-us/library/windows/hardware/ff569191(v=vs.85).aspx for algorithms
// Title: Processing in the 8-bit YUV Color Space

void    img::yuv::detail::calc_y_factors( float& m, float& b, float brightness, float contrast )
{
    m = 1.0f, b = 0;        // contrast == 0
    if( contrast > 0.0f )
    {
        m = 1.0f + contrast;
        b = contrast / -2.0f;
    }
    else if( contrast < 0.0f )
    {
        m = 1.0f + contrast * 0.5f;
        b = -0.25f * contrast;
    }

    b = b + brightness;
}


#if 0

// u,v e [0.0;1.0]
// saturation e [0.0;4.0]
// hue e [-1.0;1.0]
static void     calc_uv_value( float& u, float& v, float saturation, float hue )
{
    float first_factor = (float)cos( hue * M_PI ) * saturation;
    float second_factor = (float)sin( hue * M_PI ) * saturation;

#if 0   // original code :
    float new_u = (u - 0.5) * first_factor + (v - 0.5) * second_factor + 0.5;
    float new_v = (v - 0.5) * first_factor + (u - 0.5) * second_factor + 0.5;
#else
    // new_u = (u - 0.5) * first_factor + (v - 0.5) * second_factor + 0.5;
    // <=>        u * first_factor - 0.5 * first_factor + v * second_factor - 0.5 * second_factor + 0.5;
    // <=>        u * first_factor + v * second_factor + 0.5 - 0.5 * first_factor - 0.5 * second_factor;
    // add = 0.5 - 0.5 * first_factor - 0.5 * second_factor
    // add = 0.5 - (first_factor + second_factor) * 0.5
    // <=>        u * first_factor + v * second_factor + add;

    float add = 0.5f - (first_factor + second_factor) * 0.5f;
    float new_u = u * first_factor + v * second_factor + add;
    float new_v = v * first_factor + u * second_factor + add;
#endif

    u = CLIP( new_u, 0.0f, 1.0f );
    v = CLIP( new_v, 0.0f, 1.0f );
}

#endif

void img::yuv::detail::calc_uv_factors( float& mul_uv, float& mul_vu, float& add, float saturation, float hue )
{
    mul_uv = (float)cos( hue * M_PI ) * saturation;
    mul_vu = (float)sin( hue * M_PI ) * saturation;

    add = 0.5f - (mul_uv + mul_vu) * 0.5f;
}

static void calc_y8_uv_factors( int& mul_uv, int& mul_vu, int& add, float saturation, float hue )
{
    saturation = CLIP( saturation, 0.0f, 3.99999f );

    float first_factor_F, second_factor_F, add_F;
    img::yuv::detail::calc_uv_factors( first_factor_F, second_factor_F, add_F, saturation, hue );

    mul_uv = int( first_factor_F * 32 );
    mul_vu = int( second_factor_F * 32 );
    mul_uv = CLIP( mul_uv, -128, 127 );
    mul_vu = CLIP( mul_vu, -128, 127 );     // clip these to this range ....
    add = int( add_F * 256 );
}

static void    calc_y8_factors( int& m, int& b, float brightness, float contrast )
{
    m = 64, b = 0;
    float m_factor, b_factor;
    img::yuv::detail::calc_y_factors( m_factor, b_factor, brightness, contrast );
    m = (int)(m_factor * 64);
    b = (int)(b_factor * 256);
}

static int      apply_y8_factors( int pixel_value, int m, int b )
{
    return (m * (int)pixel_value) / 64 + b;
}

static byte     apply_y8_factors_clipped( byte pixel_value, int m, int b )
{
    int res = apply_y8_factors( pixel_value, m, b );
    return (byte)CLIP( res, 0, 256 - 1 );
}

float img::yuv::calc_y_pixel_value( float pixel, float brightness, float contrast )
{
    float m, b;
    detail::calc_y_factors( m, b, brightness, contrast );
    float val = m * pixel + b;
    return CLIP( val, 0.0f, 1.0f );
}



static void	apply_y_params_y8_c_v0( void* base_y_plane_ptr, int stride, int width, int height, float brightness, float contrast )
{
    int m, b;
    calc_y8_factors( m, b, brightness, contrast );

    byte* base_ptr = (byte*)base_y_plane_ptr;
    for( int y = 0; y < height; ++y )
    {
        byte* base_ptr_line = reinterpret_cast<byte*>(base_ptr + y * stride);

        for( int x = 0; x < width; ++x )
        {
            base_ptr_line[x] = apply_y8_factors_clipped( base_ptr_line[x], m, b );
        }
    }
}


FORCEINLINE
static void apply_uv8_c_step( uint8_t* ptr_u, uint8_t* ptr_v, int pixel_count, int mul_uv, int mul_vu, int add )
{
    for( int idx = 0; idx < pixel_count; ++idx )
    {
        int u = ptr_u[idx];
        int v = ptr_v[idx];

        int new_u = (u * mul_uv + v * mul_vu) / 32 + add;
        int new_v = (v * mul_uv + u * mul_vu) / 32 + add;

        ptr_u[idx] = (uint8_t)CLIP( new_u, 0, 256 - 1 );
        ptr_v[idx] = (uint8_t)CLIP( new_v, 0, 256 - 1 );
    }
}

void	img::yuv::detail::apply_uv_params_c_v0( byte* base_ptr, int pixel_count, float saturation, float hue )
{
    int mul_uv, mul_vu, add;
    calc_y8_uv_factors( mul_uv, mul_vu, add, saturation, hue );

	for( int i = 0; i < pixel_count; ++i )
	{
        apply_uv8_c_step( base_ptr + pixel_count * 1, base_ptr + pixel_count * 2, pixel_count, mul_uv, mul_vu, add );
	}
}


static void	apply_uv_params_y8_c_v0( void* base_y_plane_ptr, int stride, int width, int height, float saturation, float hue )
{
    int mul_uv, mul_vu, add;
    calc_y8_uv_factors( mul_uv, mul_vu, add, saturation, hue );

    byte* base_ptr = (byte*)base_y_plane_ptr;
    int plane_size = height * stride;

    for( int y = 0; y < height; ++y )
    {
        byte* base_ptr_line_u = reinterpret_cast<byte*>(base_ptr + plane_size * 1 + y * stride);
        byte* base_ptr_line_v = reinterpret_cast<byte*>(base_ptr + plane_size * 2 + y * stride);

        apply_uv8_c_step( base_ptr_line_u, base_ptr_line_v, width, mul_uv, mul_vu, add );
    }
}

static void apply_uv_params_y8( img::img_descriptor& dst, float saturation, float hue, unsigned cpu_features )
{
    using namespace img::yuv::detail;

    if( dst.type == FOURCC_YUV8PLANAR )
    {
        apply_uv_params_y8_c_v0( dst.pData, dst.pitch, dst.dim_x, dst.dim_y, saturation, hue );
    }
}

void img::yuv::apply_uv_params( img::img_descriptor& dst, float saturation, float hue, unsigned cpu_features )
{
    using namespace img::yuv::detail;

    if( saturation == 1.0f && hue == 0 )
        return;

    if( dst.type == FOURCC_YUV8PLANAR )
        apply_uv_params_y8( dst, saturation, hue, cpu_features );

}

void img::yuv::apply_y_params( img::img_descriptor& dst, float brightness, float contrast, unsigned cpu_features )
{
    using namespace img::yuv::detail;

    if( contrast == 0 && brightness == 0 )
        return;

    if( dst.type == FOURCC_Y800 || dst.type == FOURCC_YUV8PLANAR )
    {
        apply_y_params_y8_c_v0( dst.pData, dst.pitch, dst.dim_x, dst.dim_y, brightness, contrast );
    }

}
