
#include "memcpy_image.h"

#include "../sse_helper/sse_utils.h"
#include "memcpy_opt.h"

static bool     is_linear_memcpy_able( img::img_descriptor& dst, const img::img_descriptor& src, int bytes_per_line )
{
    if( dst.pitch == src.pitch && dst.data_length >= src.data_length && src.pitch > 0 )
    {
        if( bytes_per_line == 0 )
            return true;
        return bytes_per_line == src.pitch;
    }
    return false;
}

static int      calc_bytes_per_line( uint32_t fcc, unsigned dim_x )
{
    return img::calc_minimum_pitch( fcc, dim_x );
}


void img::memcpy_image( img_descriptor& dst, const img_descriptor& src, unsigned int cpu_features )
{
    memcpy_image( dst, src, false, cpu_features );
}

void img::memcpy_image( img::img_descriptor& dst, const img::img_descriptor& src, bool bFlip, unsigned int cpu_features )
{
	// we can only do direct copies here ...
	if( src.type != dst.type || src.dim_x != dst.dim_x || src.dim_y != dst.dim_y )
		return;

    bool flip = (src.pitch < 0 && dst.pitch > 0) || (src.pitch > 0 && dst.pitch < 0) ^ bFlip;


    int bytes_per_line = calc_bytes_per_line( src.type, src.dim_x );
    if( is_linear_memcpy_able( dst, src, bytes_per_line ) && !flip)
    {
        memcpy_opt_test( dst.pData, src.pData, src.data_length, cpu_features );
        return;
    }

    if( src.type == FOURCC_YUV16PLANAR || src.type == FOURCC_YUV8PLANAR || src.type == FOURCC_YUVFLOATPLANAR )
    {
        int src_plane_size = calc_plane_pitch( src );
        int dst_plane_size = calc_plane_pitch( dst );
        int bytes_per_line = calc_bytes_per_line( src.type, src.dim_x );

        // now copy all 3 planes
        memcpy_image( dst.pData + dst_plane_size * 0, dst.pitch, src.pData + src_plane_size * 0, src.pitch, bytes_per_line, src.dim_y, flip, cpu_features );
        memcpy_image( dst.pData + dst_plane_size * 1, dst.pitch, src.pData + src_plane_size * 1, src.pitch, bytes_per_line, src.dim_y, flip, cpu_features );
        memcpy_image( dst.pData + dst_plane_size * 2, dst.pitch, src.pData + src_plane_size * 2, src.pitch, bytes_per_line, src.dim_y, flip, cpu_features );
    }
    else
    {
	    if( bytes_per_line != 0 )
	    {
            bytes_per_line = MIN( MIN( bytes_per_line, (int)dst.pitch ), (int) src.pitch );
	    }
	    else
	    {
		    bytes_per_line = MIN( dst.pitch, src.pitch );
	    }
	    memcpy_image( dst.pData, dst.pitch, src.pData, src.pitch, bytes_per_line, src.dim_y, flip, cpu_features );
    }
}

void img::memcpy_image( byte* dst_ptr, int dst_pitch, byte* src_ptr, int src_pitch, int bytes_per_line, int dim_y, bool bFlip, unsigned int cpu_features )
{
	if( bFlip )
    {
		sse_utils::flip_image_params( src_ptr, src_pitch, dim_y );
    }

	if( src_pitch != dst_pitch || src_pitch < 0 || src_pitch != bytes_per_line )
	{
		for( int y = 0; y < dim_y; ++y )
		{
			memcpy_opt_test( dst_ptr + dst_pitch * y, src_ptr + src_pitch * y, bytes_per_line, cpu_features );
		}
	}
	else // src_pitch == dst_pitch && pitch > 0 && pitch == bytes_per_line
	{
		memcpy_opt_test( dst_ptr, src_ptr, dim_y * bytes_per_line, cpu_features );
	}
}
