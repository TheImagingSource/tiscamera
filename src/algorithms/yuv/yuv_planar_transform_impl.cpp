// test_yuv.cpp : Defines the entry point for the console application.
//

#include "yuv_planar_transform_impl.h"
#include "yuv_transform_common.h"

#include "../sse_helper/sse_utils.h"

#include <emmintrin.h>

#pragma warning ( disable : 4127 )	// warning C4127: conditional expression is constant

using namespace sse_utils;


void img::yuv::transform_RGB32_to_YUV8planar( img::img_descriptor& dest, const img::img_descriptor& src, unsigned /*cpu_features*/ )
{
	// detail::transform_RGB32_to_YUV8planar_sse2_v0( dest.pData, src.pData, src.pitch, src.dim_x, src.dim_y );
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//
// transform_YUV8planar_to_RGB32 code
//
//////////////////////////////////////////////////////////////////////////

void	img::yuv::detail::copy_yuv8planar_uv_planes( byte* dst_ptr, byte* src_ptr, int plane_size )
{
	memcpy( dst_ptr + plane_size * 1, src_ptr + plane_size * 1, plane_size ); // copy u plane
	memcpy( dst_ptr + plane_size * 2, src_ptr + plane_size * 2, plane_size ); // copy v plane
}

void img::yuv::detail::copy_yuv8planar_uv_planes( byte* dst_ptr, byte* src_ptr, int width, int height )
{
	copy_yuv8planar_uv_planes( dst_ptr, src_ptr, width * height );
}

void img::yuv::copy_yuv8planar_uv_planes( img::img_descriptor& dst, const img::img_descriptor& src )
{
    ASSERT( dst.dim_x == src.dim_x );
    ASSERT( dst.dim_y == src.dim_y );
    ASSERT( dst.pitch == src.pitch );
    int plane_size = src.dim_x * src.dim_y;
    detail::copy_yuv8planar_uv_planes( dst.pData, src.pData, plane_size );
}

void img::yuv::copy_yuv16planar_uv_planes( img::img_descriptor& dst, const img::img_descriptor& src )
{
    ASSERT( dst.dim_x == src.dim_x );
    ASSERT( dst.dim_y == src.dim_y );
    ASSERT( dst.pitch == src.pitch );
    int plane_size = src.dim_x * src.dim_y * 2;

    byte* dst_ptr = dst.pData;
    byte* src_ptr = src.pData;

    memcpy( dst_ptr + plane_size * 1, src_ptr + plane_size * 1, plane_size ); // copy u plane
    memcpy( dst_ptr + plane_size * 2, src_ptr + plane_size * 2, plane_size ); // copy v plane
}

void img::yuv::transform_YUV8planar_to_dest( img::img_descriptor& dest, const img::img_descriptor& src, unsigned /*cpu_features*/ )
{
    ASSERT( src.type == FOURCC_YUV8PLANAR );
    ASSERT( dest.type == FOURCC_RGB32 || dest.type == FOURCC_RGB24 );

    // if( dest.type == FOURCC_RGB32 )
    //     detail::transform_YUV8planar_to_RGB32_sse2( dest.pData, dest.pitch, src.pData, src.dim_x, src.dim_y );
    // else if( dest.type == FOURCC_RGB24 )
    //     detail::transform_YUV8planar_to_RGB24_ssse3( dest.pData, dest.pitch, src.pData, src.dim_x, src.dim_y );


}

void img::yuv::copy_yuv_planar_uv_planes( img::img_descriptor& dst, const img::img_descriptor& src )
{
    ASSERT( dst.type == FOURCC_YUV8PLANAR || dst.type == FOURCC_YUV16PLANAR );
    ASSERT( dst.type == src.type );
    ASSERT( dst.dim_x == src.dim_x );
    ASSERT( dst.dim_y == src.dim_y );

    if( dst.type == FOURCC_YUV8PLANAR )
        copy_yuv16planar_uv_planes( dst, src );
    else if( dst.type == FOURCC_YUV16PLANAR )
        copy_yuv16planar_uv_planes( dst, src );
}
