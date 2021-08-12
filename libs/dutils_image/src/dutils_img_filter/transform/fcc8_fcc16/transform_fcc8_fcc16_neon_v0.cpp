
#include "transform_fcc8_fcc16_internal.h"

#include "../../simd_helper/use_simd_A64.h"

namespace
{
    void transform_fcc8_to_fcc16_neon_v0( img::img_descriptor dst, img::img_descriptor src )
    {
        // NOTE: a version using vzip seems to be slower then this ...

        uint8x8_t reg_lo = INIT_Neon_u8x8( 0xFF, 0x00, 0xFF, 0x01, 0xFF, 0x02, 0xFF, 0x03 );
        uint8x8_t reg_hi = INIT_Neon_u8x8( 0xFF, 0x04, 0xFF, 0x05, 0xFF, 0x06, 0xFF, 0x07 );

        for( int y = 0; y < src.dim.cy; ++y )
        {
            auto* src_line = img::get_line_start<const uint8_t>( src, y );
            auto* dst_line = img::get_line_start<uint16_t>( dst, y );

            int x = 0;
            for( ; x < (dst.dim.cx - 7); x += 8 )
            {
                auto reg = vld1_u8( src_line + x );

                auto lo = vtbl1_u8( reg, reg_lo );
                auto hi = vtbl1_u8( reg, reg_hi );

                vst1_u8( reinterpret_cast<uint8_t*>(dst_line + x + 0), lo );
                vst1_u8( reinterpret_cast<uint8_t*>(dst_line + x + 4), hi );
            }
            img_filter::transform::internal::transform_fcc8_to_fcc16_c_line( src_line + x, dst_line + x, src.dim.cx - x );
        }
    }

#if 0
    void    transform_fcc16_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
    {
        for( int y = 0; y < src.dim.cy; ++y )
        {
            auto* src_line = img::get_line_start<const uint16_t>( src, y );
            auto* dst_line = img::get_line_start<uint8_t>( dst, y );

            int x = 0;
            for( ; x < (dst.dim.cx - 7); x += 8 )
            {
                auto t0 = vld2_u8( reinterpret_cast<const uint8_t*>(src_line + x + 0) );

                vst1_u8( dst_line + x + 0, t0.val[1] );
            }
            img_filter::transform::internal::transform_fcc16_to_fcc8_c_line( src_line + x, dst_line + x, src.dim.cx - x );
        }
    }
#else
    void    transform_fcc16_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
    {
        for( int y = 0; y < src.dim.cy; ++y )
        {
            auto* src_line = img::get_line_start<const uint16_t>( src, y );
            auto* dst_line = img::get_line_start<uint8_t>( dst, y );

            int x = 0;
            for( ; x < (dst.dim.cx - 31); x += 32 )
            {
                auto t0 = vld2q_u8( reinterpret_cast<const uint8_t*>(src_line + x + 0) );
                auto t1 = vld2q_u8( reinterpret_cast<const uint8_t*>(src_line + x + 16) );

                vst1q_u8( dst_line + x + 0, t0.val[1] );
                vst1q_u8( dst_line + x + 16, t1.val[1] );
            }

            for( ; x < (dst.dim.cx - 15); x += 16 )
            {
                auto t0 = vld2q_u8( reinterpret_cast<const uint8_t*>(src_line + x + 0) );
                vst1q_u8( dst_line + x + 0, t0.val[1] );
            }

            for( ; x < (dst.dim.cx - 7); x += 8 )
            {
                auto t0 = vld2_u8( reinterpret_cast<const uint8_t*>(src_line + x + 0) );

                vst1_u8( dst_line + x + 0, t0.val[1] );
            }

            img_filter::transform::internal::transform_fcc16_to_fcc8_c_line( src_line + x, dst_line + x, src.dim.cx - x );
        }
    }
#endif

    FORCEINLINE
    uint8x16_t	    wb_by8_step_( uint8x16_t src, uint16x8_t mul )
    {
        uint16x8_t lo = vmovl_u8( vget_low_u8( src ) );
        uint16x8_t hi = vmovl_high_u8( src );

        uint8x8_t res_lo = vqshrn_n_u16( lo * mul, 6 );
        uint8x8_t res_hi = vqshrn_n_u16( hi * mul, 6 );

        return vcombine_u8( res_lo, res_hi );
    }

    FORCEINLINE
    void    transform_fcc16_to_fcc8_wb_neon_step32( uint8_t* dst_pos, const uint16_t* src_pos, uint16x8_t mul )
    {
        auto t0 = vld2q_u8( reinterpret_cast<const uint8_t*>(src_pos + 0) );
        auto t1 = vld2q_u8( reinterpret_cast<const uint8_t*>(src_pos + 16) );

        auto r0 = wb_by8_step_( t0.val[1], mul );
        auto r1 = wb_by8_step_( t1.val[1], mul );

        vst1q_u8( dst_pos + 0, r0 );
        vst1q_u8( dst_pos + 16, r1 );
    }

    FORCEINLINE
    void transform_fcc16_to_fcc8_wb_neon_line( uint8_t* dst_line, const uint16_t* src_line, int width, uint16x8_t mul, uint16x8_t mul_odd_pixel )
    {
        int x = 0;

        for( ; x <= (width - 32); x += 32 )
        {
            transform_fcc16_to_fcc8_wb_neon_step32( dst_line + x, src_line + x, mul );
        }

        if( x != width )
        {
            if( x % 2 )
            {
                transform_fcc16_to_fcc8_wb_neon_step32( dst_line + width - 32, src_line + width - 32, mul_odd_pixel );
            }
            else
            {
                transform_fcc16_to_fcc8_wb_neon_step32( dst_line + width - 32, src_line + width - 32, mul );
            }
        }
    }

    void transform_fcc16_to_fcc8_wb_neon_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params )
    {
        assert( dst.dim.cx % 2 == 0 );
        assert( dst.dim.cx >= 32 );

        img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

        auto wb_x0y0 = static_cast<uint8_t>(wb_params.wb_x0y0 * (64));
        auto wb_x1y0 = static_cast<uint8_t>(wb_params.wb_x1y0 * (64));
        auto wb_x0y1 = static_cast<uint8_t>(wb_params.wb_x0y1 * (64));
        auto wb_x1y1 = static_cast<uint8_t>(wb_params.wb_x1y1 * (64));

        const uint16x8_t wb_even_line = uint16x8_t{ wb_x0y0, wb_x1y0, wb_x0y0, wb_x1y0, wb_x0y0, wb_x1y0, wb_x0y0, wb_x1y0 };
        const uint16x8_t wb_even_line_odd_pixel = uint16x8_t{ wb_x1y0, wb_x0y0, wb_x1y0, wb_x0y0, wb_x1y0, wb_x0y0, wb_x1y0, wb_x0y0 };
        const uint16x8_t wb_odd_line = uint16x8_t{ wb_x0y1, wb_x1y1, wb_x0y1, wb_x1y1, wb_x0y1, wb_x1y1, wb_x0y1, wb_x1y1 };
        const uint16x8_t wb_odd_line_odd_pixel = uint16x8_t{ wb_x1y1, wb_x0y1, wb_x1y1, wb_x0y1, wb_x1y1, wb_x0y1, wb_x1y1, wb_x0y1 };

        for( int y = 0; y < src.dim.cy; y += 2 )
        {
            auto* src_line0 = img::get_line_start<const uint16_t>( src, y + 0 );
            auto* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );
            transform_fcc16_to_fcc8_wb_neon_line( dst_line0, src_line0, dst.dim.cx, wb_even_line, wb_even_line_odd_pixel );

            auto* src_line1 = img::get_line_start<const uint16_t>( src, y + 1 );
            auto* dst_line1 = img::get_line_start<uint8_t>( dst, y + 1 );
            transform_fcc16_to_fcc8_wb_neon_line( dst_line1, src_line1, dst.dim.cx, wb_odd_line, wb_odd_line_odd_pixel );
        }
    }
}

auto img_filter::transform::get_transform_fcc8_to_fcc16_neon( const img::img_type& dst, const img::img_type& src ) -> transform_function_type
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }
    if( can_convert_fcc8_to_fcc16( dst, src ) ) {
        return transform_fcc8_to_fcc16_neon_v0;
    }
    return nullptr;
}

auto img_filter::transform::get_transform_fcc16_to_fcc8_neon( const img::img_type& dst, const img::img_type& src ) -> transform_function_type
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }
    if( can_convert_fcc16_to_fcc8( dst, src ) ) {
        return transform_fcc16_to_fcc8_neon_v0;
    }
    return nullptr;
}

auto img_filter::transform::get_transform_fcc16_to_fcc8_wb_neon( const img::img_type& dst, const img::img_type& src ) -> transform_function_param_type
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }
    if( src.dim.cx < 32 ) {
        return nullptr;
    }
    if( can_convert_fcc16_to_fcc8( dst, src ) ) {
        return transform_fcc16_to_fcc8_wb_neon_v0;
    }
    return nullptr;
}