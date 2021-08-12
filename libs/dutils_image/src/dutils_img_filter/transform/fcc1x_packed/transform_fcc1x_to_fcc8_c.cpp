
#include "transform_fcc1x_to_fcc8.h"

#include "fcc1x_packed_to_fcc8_internal_loop.h"
#include "fcc1x_packed_to_fcc16_internal_loop.h"

using namespace fcc1x_packed_internal;
using filter_params = img_filter::filter_params;

namespace
{
inline uint8_t  apply_u16_wb_to_u8_c( uint16_t val, int fac ) noexcept
{
    const auto res = (val * fac) >> (6 + 8);      // float_fac * 64 / 64
    return (res > 0xFF) ? 0xFF : static_cast<uint8_t>(res);
}

template<auto func>
inline void     apply_wb_line_c( int width, uint8_t* dst_line0, const void* src_line0, int wb_x0, int wb_x1 )
{
    int x = 0;
    for( ; x < (width - 1); x += 2 )
    {
        dst_line0[x + 0] = apply_u16_wb_to_u8_c( func( src_line0, x + 0 ), wb_x0 );
        dst_line0[x + 1] = apply_u16_wb_to_u8_c( func( src_line0, x + 1 ), wb_x1 );
    }
    if( width % 2 ) {
        dst_line0[x] = apply_u16_wb_to_u8_c( func( src_line0, x ), wb_x0 );
    }
}

template<auto func>
inline void     apply_wb_lines_c( const img::img_descriptor& dst, const img::img_descriptor& src, const img_filter::bayer_pattern_parameters& wb_params )
{
    int wb_x0y0 = static_cast<int>(wb_params.wb_x0y0 * 64);
    int wb_x1y0 = static_cast<int>(wb_params.wb_x1y0 * 64); // mul to 1.f ^= 64
    int wb_x0y1 = static_cast<int>(wb_params.wb_x0y1 * 64);
    int wb_x1y1 = static_cast<int>(wb_params.wb_x1y1 * 64);

    int y = 0;
    for( ; y < (src.dim.cy - 1); y += 2 )
    {
        auto* src_line0 = img::get_line_start<const uint8_t>( src, y + 0 );
        auto* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );
        apply_wb_line_c<func>( src.dim.cx, dst_line0, src_line0, wb_x0y0, wb_x1y0 );

        auto* src_line1 = img::get_line_start<const uint8_t>( src, y + 1 );
        auto* dst_line1 = img::get_line_start<uint8_t>( dst, y + 1 );
        apply_wb_line_c<func>( src.dim.cx, dst_line1, src_line1, wb_x0y1, wb_x1y1 );
    }
    if( y == (src.dim.cy - 1) )
    {
        auto* src_line0 = img::get_line_start<const uint8_t>( src, y + 0 );
        auto* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );
        apply_wb_line_c<func>( src.dim.cx, dst_line0, src_line0, wb_x0y0, wb_x1y0 );
    }
}

void transform_wb_fcc12_to_dst8_c( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params )
{
    assert( dst.dim == src.dim );

    img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

    apply_wb_lines_c<&calc_fcc12_to_fcc16>( dst, src, wb_params );
}

void transform_wb_fcc12_mipi_to_dst8_c( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 2 == 0 );

    img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

    apply_wb_lines_c<&calc_fcc12_mipi_to_fcc16>( dst, src, wb_params );
}

void transform_wb_fcc12_packed_to_dst8_c( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 2 == 0 );

    img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

    apply_wb_lines_c<&calc_fcc12_packed_to_fcc16>( dst, src, wb_params );
}

void transform_wb_fcc12_spacked_to_dst8_c( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 2 == 0 );

    img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

    apply_wb_lines_c<&calc_fcc12_spacked_to_fcc16>( dst, src, wb_params );
}

void transform_wb_fcc10_to_dst8_c( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params )
{
    assert( dst.dim == src.dim );

    img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

    apply_wb_lines_c<&calc_fcc10_to_fcc16>( dst, src, wb_params );
}

void transform_wb_fcc10_spacked_to_dst8_c( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 4 == 0 );

    img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

    apply_wb_lines_c<&calc_fcc10_spacked_to_fcc16>( dst, src, wb_params );
}

void transform_wb_fcc10_mipi_to_dst8_c( const img::img_descriptor& dst, const img::img_descriptor& src, filter_params& params )
{
    assert( dst.dim == src.dim );
    assert( dst.dim.cx % 4 == 0 );

    img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

    apply_wb_lines_c<&calc_fcc10_packed_mipi_to_fcc16>( dst, src, wb_params );
}

}

auto img_filter::transform::fcc1x_packed::get_transform_fcc1x_to_fcc8_c( const img::img_type& dst, const img::img_type& src ) -> transform_function_param_type
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
    case fccXX_pack_type::fcc12:            return ::transform_wb_fcc12_to_dst8_c;
    case fccXX_pack_type::fcc12_mipi:       return ::transform_wb_fcc12_mipi_to_dst8_c;
    case fccXX_pack_type::fcc12_packed:     return ::transform_wb_fcc12_packed_to_dst8_c;
    case fccXX_pack_type::fcc12_spacked:    return ::transform_wb_fcc12_spacked_to_dst8_c;

    case fccXX_pack_type::fcc10:            return ::transform_wb_fcc10_to_dst8_c;
    case fccXX_pack_type::fcc10_spacked:    return ::transform_wb_fcc10_spacked_to_dst8_c;
    case fccXX_pack_type::fcc10_mipi:       return ::transform_wb_fcc10_mipi_to_dst8_c;
    
    case fccXX_pack_type::invalid:          return nullptr;
    };
    return nullptr;
}
