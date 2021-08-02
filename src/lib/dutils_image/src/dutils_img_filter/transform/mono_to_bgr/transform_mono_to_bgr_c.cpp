
#include "transform_mono_to_bgr.h"
#include "transform_mono_to_bgr_internal.h"

#include <dutils_img/pixel_structs.h>

namespace
{

using namespace transform_mono_to_bgr_internal;
using namespace img::pixel_type;

void transform_mono8_to_bgr24_c( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( dst.dim == src.dim );
    assert( dst.fourcc_type() == img::fourcc::BGR24 && src.fourcc_type() == img::fourcc::MONO8 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGR24>( dst, y );

        transform_MONO8_to_BGR24_c_line( dst.dim.cx, src_line, dst_line );
    }
}

void transform_mono8_to_bgra32_c( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( dst.dim == src.dim );
    assert( dst.fourcc_type() == img::fourcc::BGRA32 && src.fourcc_type() == img::fourcc::MONO8 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA32>( dst, y );

        transform_MONO8_to_BGRA32_c_line( dst.dim.cx, src_line, dst_line );
    }
}

void transform_mono8_to_bgra64_c( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( dst.dim == src.dim );
    assert( dst.fourcc_type() == img::fourcc::BGRA64 && src.fourcc_type() == img::fourcc::MONO8 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA64>( dst, y );

        transform_MONO8_to_BGRA64_c_line( dst.dim.cx, src_line, dst_line );
    }
}

void transform_mono16_to_bgr24_c( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( dst.dim == src.dim );
    assert( dst.fourcc_type() == img::fourcc::BGR24 && src.fourcc_type() == img::fourcc::MONO16 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<BGR24>( dst, y );

        transform_MONO16_to_BGR24_c_line( dst.dim.cx, src_line, dst_line );
    }
}

void transform_mono16_to_bgra32_c( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( dst.dim == src.dim );
    assert( dst.fourcc_type() == img::fourcc::BGRA32 && src.fourcc_type() == img::fourcc::MONO16 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA32>( dst, y );

        transform_MONO16_to_BGRA32_c_line( dst.dim.cx, src_line, dst_line );
    }
}

void transform_mono16_to_bgra64_c( img::img_descriptor dst_, img::img_descriptor src )
{
    auto dst = img::flip_image_in_img_desc_if_allowed( dst_ );

    assert( dst.dim == src.dim );
    assert( dst.fourcc_type() == img::fourcc::BGRA64 && src.fourcc_type() == img::fourcc::MONO16 );

    for( int y = 0; y < dst.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<BGRA64>( dst, y );

        transform_MONO16_to_BGRA64_c_line( dst.dim.cx, src_line, dst_line );
    }
}

}


img_filter::transform_function_type     img_filter::transform::get_transform_mono_to_bgr_c( const img::img_type& dst, const img::img_type& src )
{
    if( dst.dim != src.dim ) {
        return nullptr;
    }

    if( src.fourcc_type() == img::fourcc::MONO8 )
    {
        switch( dst.fourcc_type() )
        {
        case img::fourcc::BGR24:        return ::transform_mono8_to_bgr24_c;
        case img::fourcc::BGRA32:       return ::transform_mono8_to_bgra32_c;
        case img::fourcc::BGRA64:       return ::transform_mono8_to_bgra64_c;
        default:
            return nullptr;
        }
    }
    if( src.fourcc_type() == img::fourcc::MONO16 )
    {
        switch( dst.fourcc_type() )
        {
        case img::fourcc::BGR24:        return ::transform_mono16_to_bgr24_c;
        case img::fourcc::BGRA32:       return ::transform_mono16_to_bgra32_c;
        case img::fourcc::BGRA64:       return ::transform_mono16_to_bgra64_c;
        default:
            return nullptr;
        }
    }
    return nullptr;
}
