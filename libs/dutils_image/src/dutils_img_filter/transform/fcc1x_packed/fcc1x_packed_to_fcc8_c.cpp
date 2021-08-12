
#include "fcc1x_packed_to_fcc.h"

#include "fcc1x_packed_to_fcc8_internal_loop.h"

using namespace fcc1x_packed_internal;

namespace
{

void transform_fcc12_packed_to_dst8_c( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 2 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

        transform_fcc12_packed_to_fcc8_c_line( src_line, dst_line, src.dim.cx );
    }
}

void transform_fcc12_packed_mipi_to_dst8_c( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 2 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

        transform_fcc12_mipi_to_fcc8_c_line( src_line, dst_line, src.dim.cx );
    }
}

void transform_fcc12_spacked_to_dst8_c( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 2 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

        transform_fcc12_spacked_to_fcc8_c_line( src_line, dst_line, src.dim.cx );
    }
}

void transform_fcc10_spacked_to_dst8_c( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 4 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint8_t* dst_line = img::get_line_start<uint8_t>( dst, y );

        transform_fcc10_spacked_to_fcc8_c_line( src_line, dst_line, src.dim.cx );
    }
}


void transform_fcc10_packed_mipi_to_dst8_c(img::img_descriptor dst, img::img_descriptor src)
{
    assert( dst.dim == src.dim );
    assert(dst.dim.cx % 4 == 0);

    for (int y = 0; y < src.dim.cy; ++y)
    {
        uint8_t* src_line = img::get_line_start<uint8_t>(src, y);
        uint8_t* dst_line = img::get_line_start<uint8_t>(dst, y);

        for (int dst_x = 0, src_x = 0; dst_x < dst.dim.cx; dst_x += 4, src_x += 5)
        {
            dst_line[dst_x + 0] = src_line[src_x + 0];
            dst_line[dst_x + 1] = src_line[src_x + 1];
            dst_line[dst_x + 2] = src_line[src_x + 2];
            dst_line[dst_x + 3] = src_line[src_x + 3];
        }
    }
}

void transform_fcc12_to_dst8_c( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim == src.dim );
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint16_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );

        for( int x = 0; x < dst.dim.cx; ++x )
        {
            dst_line[x] = calc_fcc12_to_fcc8( src_line, x );
        }
    }
}

void transform_fcc10_to_dst8_c( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim == src.dim );
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint16_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );

        for( int x = 0; x < dst.dim.cx; ++x )
        {
            dst_line[x] = calc_fcc10_to_fcc8( src_line, x );
        }
    }
}
}

auto img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_c( const img::img_type& dst, const img::img_type& src ) -> transform_function_type
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }

    using namespace img::fcc1x_packed;

    if( !is_accepted_dst_fcc8( dst.fourcc_type() ) ) {
        return nullptr;
    }

    switch( get_fcc1x_pack_type( src.fourcc_type() ) )
    {
    case fccXX_pack_type::fcc12:            return ::transform_fcc12_to_dst8_c;
    case fccXX_pack_type::fcc12_packed:     return ::transform_fcc12_packed_to_dst8_c;
    case fccXX_pack_type::fcc12_mipi:       return ::transform_fcc12_packed_mipi_to_dst8_c;
    case fccXX_pack_type::fcc12_spacked:    return ::transform_fcc12_spacked_to_dst8_c;

    case fccXX_pack_type::fcc10:            return ::transform_fcc10_to_dst8_c;
    case fccXX_pack_type::fcc10_spacked:    return ::transform_fcc10_spacked_to_dst8_c;
    case fccXX_pack_type::fcc10_mipi:       return ::transform_fcc10_packed_mipi_to_dst8_c;
    
    case fccXX_pack_type::invalid:          return nullptr;
    };
    return nullptr;
}