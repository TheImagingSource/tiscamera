
#include "memcpy_image.h"

#include <dutils_img/dutils_img.h>
#include <dutils_img_lib/dutils_img_helper.h>

#include "alignment_helper.h"
#include "interop_private.h"

#include <algorithm>

#include <cstring>

namespace
{

static FORCEINLINE void     internal_memcpy( void* dst, void* src, int bytes ) noexcept
{
    memcpy( dst, src, (size_t) bytes );
}

inline void	flip_image_params( uint8_t*& ptr, int& pitch, int dim_y ) noexcept
{
    ptr = (ptr + pitch * (dim_y - 1));
    pitch = -pitch;
}

inline img::img_plane	flip_image_params( img::img_plane src, int dim_y ) noexcept
{
    return {
        static_cast<uint8_t*>( src.plane_ptr ) + src.pitch * (dim_y - 1),
        -src.pitch
    };
}

static bool     is_linear_memcpy_able( const img::img_descriptor& dst, const img::img_descriptor& src, int bytes_per_line ) noexcept
{
    if( dst.pitch() != src.pitch() || src.pitch() < 0 ) {
        return false;
    }
    if( bytes_per_line == 0 ) {
        return true;
    }
    return bytes_per_line == src.pitch();
}

static void copy_image_lines( uint8_t* dst_ptr, int dst_pitch, uint8_t* src_ptr, int src_pitch, int bytes_per_line, int dim_y ) noexcept
{
    for( int y = 0; y < dim_y; ++y )
    {
        internal_memcpy( dst_ptr + dst_pitch * y, src_ptr + src_pitch * y, bytes_per_line );
    }
}
}

void img::memcpy_image( const img::img_descriptor& dst, const img::img_descriptor& src ) noexcept
{
    if( src.type != dst.type || src.dimensions() != dst.dimensions() ) {
        return;
    }

    if( src.pitch() == 0 && dst.pitch() == 0 ) {
        auto min_len = std::min( src.data_length, dst.data_length );
        internal_memcpy( dst.data(), src.data(), min_len );
        return;
    }

    if( img::is_multi_plane_format( src.fourcc_type() ) )
    {
        const auto plane_info = img::planar::get_fcc_info( src.fourcc_type() );
        for( int plane_idx = 0; plane_idx < plane_info.plane_count; ++plane_idx )
        {
            int bytes_per_line = img::planar::get_plane_pitch_minimum( src.fourcc_type(), src.dim.cx, plane_idx );
            memcpy_image( dst.plane( plane_idx ), src.plane( plane_idx ), src.dim.cy, bytes_per_line );
        }
    }
    else
    {
        const int bytes_per_line = img::calc_minimum_pitch( src.fourcc_type(), src.dim.cx );  // this could be smaller then src.pitch
        if( is_linear_memcpy_able( dst, src, bytes_per_line ) )
        {
            auto len_to_copy = std::min( src.data_length, dst.data_length );
            internal_memcpy( dst.data(), src.data(), len_to_copy );
            return;
        }

        copy_image_lines( dst.data(), dst.pitch(), src.data(), src.pitch(), bytes_per_line, dst.dim.cy );
    }
}

void    img::memcpy_image( img_plane dst, img_plane src, int dim_y, int bytes_per_line ) noexcept
{
    assert( bytes_per_line >= 0 );
    if( dim_y < 0 ) {
        dim_y = -dim_y;
        dst = flip_image_params( dst, dim_y );
    }

    if( dst.pitch == src.pitch && dst.pitch == bytes_per_line ) {
        internal_memcpy( dst.plane_ptr, src.plane_ptr, dim_y * bytes_per_line );
    }
    else
    {
        for( int y = 0; y < dim_y; ++y )
        {
            internal_memcpy( static_cast<uint8_t*>( dst.plane_ptr ) + dst.pitch * y, static_cast<uint8_t*>( src.plane_ptr ) + src.pitch * y, bytes_per_line );
        }
    }
}

void img::memcpy_image( void* dst_ptr, int dst_pitch, void* src_ptr, int src_pitch, int bytes_per_line, int dim_y, bool bFlip ) noexcept
{
    uint8_t* actual_src_parameter = (uint8_t*)src_ptr;
    if( bFlip )
    {
        flip_image_params( actual_src_parameter, src_pitch, dim_y );
    }

    if( src_pitch != dst_pitch || src_pitch < 0 || src_pitch != bytes_per_line )
    {
        copy_image_lines( (uint8_t*)dst_ptr, dst_pitch, actual_src_parameter, src_pitch, bytes_per_line, dim_y );
    }
    else // src_pitch == dst_pitch && pitch > 0 && pitch == bytes_per_line
    {
        internal_memcpy( dst_ptr, actual_src_parameter, dim_y * bytes_per_line );
    }
}

void img::fill_image( const img_descriptor& data, uint8_t byte_value ) noexcept
{
    int bytes_to_fill_per_line = (img::get_bits_per_pixel( data.type ) * data.dim.cx) / 8;
    for( int y = 0; y < data.dim.cy; ++y )
    {
        uint8_t* line = img::get_line_start( data, y );
        memset( line, byte_value, static_cast<size_t>(bytes_to_fill_per_line) );
    }
}

void img_lib::helper::memcpy_image( const img::img_descriptor& dst, const img::img_descriptor& src ) noexcept
{
    img::memcpy_image( dst, src );
}