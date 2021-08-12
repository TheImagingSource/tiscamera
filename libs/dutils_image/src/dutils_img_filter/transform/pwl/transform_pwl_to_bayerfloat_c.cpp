
#include "../fcc1x_packed/fcc1x_packed_to_fcc16_internal.h"
#include "transform_pwl_to_bayerfloat_internal.h"

namespace
{
    void    transform_pwl12_mipi_to_bayerfloat_line_c_v0( const uint8_t* src_line, float* dst_line, int dim_x, const float* lut )
    {
        for( int x = 0; x < (dim_x - 1); x += 2 )
        {
            const auto [v0,v1] = fcc1x_packed_internal::calc_fcc12_mipi_to_fcc12_2x_pix_str( src_line, x );

            dst_line[x + 0] = lut[v0];
            dst_line[x + 1] = lut[v1];
        }
        if( dim_x % 2 ) {
            auto val = fcc1x_packed_internal::calc_fcc12_mipi_to_fcc12( src_line, dim_x - 1 );
            dst_line[dim_x - 1] = lut[val];
        }
    }

    void    transform_pwl12_to_bayerfloat_line_c_v0( const uint16_t* src_line, float* dst_line, int dim_x, const float* lut )
    {
        for( int x = 0; x < dim_x; ++x )
        {
            uint16_t val0 = src_line[x];
            assert( val0 < 0x1000 );
            dst_line[x] = lut[val0];
        }
    }

    void    transform_pwl16H12_to_bayerfloat_line_c_v0( const uint16_t* src_line, float* dst_line, int dim_x, const float* lut )
    {
        for( int x = 0; x < dim_x; ++x )
        {
            uint16_t val0 = fcc1x_packed_internal::calc_fcc16H12_to_fcc12( src_line, x );
            assert( val0 < 0x1000 );
            dst_line[x] = lut[val0];
        }
    }

}

void    img_filter::transform::pwl::detail::transform_pwl12_mipi_to_fccfloat_c_v0( img::img_descriptor dst, img::img_descriptor src )
{
    auto lut = transform_pwl_internal::get_lut_for_transform_pwl_to_float();
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint8_t>( src, y );
        auto* dst_line = img::get_line_start<float>( dst, y );

        transform_pwl12_mipi_to_bayerfloat_line_c_v0( src_line, dst_line, src.dim.cx, lut );
    }
}

void    img_filter::transform::pwl::detail::transform_pwl12_to_fccfloat_c_v0( img::img_descriptor dst, img::img_descriptor src )
{
    auto lut = transform_pwl_internal::get_lut_for_transform_pwl_to_float();
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint16_t>( src, y );
        auto* dst_line = img::get_line_start<float>( dst, y );

        transform_pwl12_to_bayerfloat_line_c_v0( src_line, dst_line, src.dim.cx, lut );
    }
}

void    img_filter::transform::pwl::detail::transform_pwl16H12_to_fccfloat_c_v0( img::img_descriptor dst, img::img_descriptor src )
{
    auto lut = transform_pwl_internal::get_lut_for_transform_pwl_to_float();
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<uint16_t>( src, y );
        auto* dst_line = img::get_line_start<float>( dst, y );

        transform_pwl16H12_to_bayerfloat_line_c_v0( src_line, dst_line, src.dim.cx, lut );
    }
}

float      img_filter::transform::pwl::calc_pwl12_pixel_to_float( const void* line_start, int pixel_offset ) noexcept
{
    auto lut = transform_pwl_internal::get_lut_for_transform_pwl_to_float();

    const uint16_t value = static_cast<const uint16_t*>(line_start)[pixel_offset];
    assert( value <= 4096 );
    return lut[value];
}

float      img_filter::transform::pwl::calc_pwl12_mipi_pixel_to_float( const void* line_start, int pixel_offset ) noexcept
{
    auto lut = transform_pwl_internal::get_lut_for_transform_pwl_to_float();

    const uint16_t value = fcc1x_packed_internal::calc_fcc12_mipi_to_fcc12( line_start, pixel_offset );
    assert( value <= 4096 );
    return lut[value];
}

float      img_filter::transform::pwl::calc_pwl16H12_pixel_to_float( const void* line_start, int pixel_offset ) noexcept
{
    auto lut = transform_pwl_internal::get_lut_for_transform_pwl_to_float();

    const uint16_t value = fcc1x_packed_internal::calc_fcc16H12_to_fcc12( line_start, pixel_offset );
    assert( value <= 4096 );
    return lut[value];
}

img_filter::transform_function_type  img_filter::transform::pwl::get_transform_pwl_to_fccfloat_c( const img::img_type& dst, const img::img_type& src )
{
    if( dst.dim != src.dim ) {
        return nullptr;
    }
    if( transform_pwl_internal::can_transform_pwl_to_fcc32f( dst, src ) )
    {
        switch( src.fourcc_type() )
        {
        case img::fourcc::PWL_RG12_MIPI:        return detail::transform_pwl12_mipi_to_fccfloat_c_v0;
        case img::fourcc::PWL_RG12:             return detail::transform_pwl12_to_fccfloat_c_v0;
        case img::fourcc::PWL_RG16H12:          return detail::transform_pwl16H12_to_fccfloat_c_v0;
        default:
            return nullptr;
        }
    }
    return nullptr;
}
