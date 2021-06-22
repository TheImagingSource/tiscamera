

#include "transform_fcc1x_to_fcc8.h"

#include "../../simd_helper/use_simd_A64.h"

#include "fcc1x_packed_to_fcc16_internal.h"
#include "fcc1x_packed_to_fcc16_internal_loop.h"
#include "../../filter/whitebalance/wb_apply.h"
#include "fcc1x_packed_to_fcc.h"

using namespace fcc1x_packed_internal;

namespace
{
    using namespace img::fcc1x_packed;


    FORCEINLINE
    uint8x8_t	    apply_wb_u8( uint8x8_t src, uint8x8_t mul ) noexcept
    {
        const uint16x8_t tmp_lo = vmull_u8( src, mul );

        return vqshrn_n_u16( tmp_lo, 6 );
    }

    template<bool is_mipi>
    FORCEINLINE
    void    transform_wb_fcc12m_or_12p_to_fcc8_neon_step_v0( uint8_t* dst_pos, const uint8_t* src_pos, uint8x8_t mul_even, uint8x8_t mul_odd )
    {
        uint8x8x3_t srcs = vld3_u8( src_pos );  // srcs.val[0] = src[0], src[3], ...; srcs.val[1] = src[1], src[4], ...;

        constexpr int idx_odd_pix = is_mipi ? 1 : 2;

        uint8x8_t src_even = srcs.val[0];
        uint8x8_t src_oddd = srcs.val[idx_odd_pix];

        uint8x8_t res_val0 = apply_wb_u8( src_even, mul_even );
        uint8x8_t res_val1 = apply_wb_u8( src_oddd, mul_odd );
        
        vst2_u8( dst_pos, uint8x8x2_t{ res_val0, res_val1 } );
    }
    
    template<bool is_fcc10>
    FORCEINLINE
    void    transform_wb_fcc10or12_to_fcc8_neon_step_v0( uint8_t* dst_pos, const uint8_t* src_pos, uint8x8_t mul_even, uint8x8_t mul_odd )
    {
        uint16x8x2_t srcs = vld2q_u16( reinterpret_cast<const uint16_t*>( src_pos ) );  // srcs.val[0] = src[0], src[3], ...; srcs.val[1] = src[1], src[4], ...;

        constexpr int shift = is_fcc10 ? 2 : 4;

        uint8x8_t src_even = vshrn_n_u16( srcs.val[0], shift );
        uint8x8_t src_oddd = vshrn_n_u16( srcs.val[1], shift );

        uint8x8_t res_val0 = apply_wb_u8( src_even, mul_even );
        uint8x8_t res_val1 = apply_wb_u8( src_oddd, mul_odd );

        vst2_u8( dst_pos, uint8x8x2_t{ res_val0, res_val1 } );
    }


    FORCEINLINE
    void    transform_wb_fcc10m_to_fcc8_neon_step_v0( uint8_t* dst_pos, const uint8_t* src_pos, uint8x8_t mul_even, uint8x8_t mul_odd )
    {
        uint32_t tmp0 = *reinterpret_cast<const uint32_t*>(src_pos + 0);
        uint32_t tmp1 = *reinterpret_cast<const uint32_t*>(src_pos + 5);
        uint32_t tmp2 = *reinterpret_cast<const uint32_t*>(src_pos + 10);
        uint32_t tmp3 = *reinterpret_cast<const uint32_t*>(src_pos + 15);

        uint32x2_t tmp_u32_0 = { tmp0, tmp1 };
        uint32x2_t tmp_u32_1 = { tmp2, tmp3 };

        uint8x8x2_t zipped = vuzp_u8( vreinterpret_u8_u32( tmp_u32_0 ), vreinterpret_u8_u32( tmp_u32_1 ) );

        uint8x8_t res_val0 = apply_wb_u8( zipped.val[0], mul_even );
        uint8x8_t res_val1 = apply_wb_u8( zipped.val[1], mul_odd );

        vst2_u8( dst_pos, uint8x8x2_t{ res_val0, res_val1 } );
    }

    template<img::fcc1x_packed::fccXX_pack_type pack_type>
    FORCEINLINE
    void transform_wb_fcc1x_to_fcc8_neon_line_v0( uint8_t* dst_line, const uint8_t* src_line, int width, uint8x8_t mul_even, uint8x8_t mul_odd )
    {
        if constexpr( pack_type == fccXX_pack_type::fcc10 || pack_type == fccXX_pack_type::fcc12 )
        {
            constexpr bool is_fcc10 = pack_type == fccXX_pack_type::fcc10;
            for( int x = 0; x < (width - 16); x += 16 )
            {
                transform_wb_fcc10or12_to_fcc8_neon_step_v0<is_fcc10>( dst_line + x, src_line + (x * 2), mul_even, mul_odd );
            }
            transform_wb_fcc10or12_to_fcc8_neon_step_v0<is_fcc10>( dst_line + width - 16, src_line + (width - 16) * 2, mul_even, mul_odd );
        }
        else if constexpr( pack_type == fccXX_pack_type::fcc12_mipi || pack_type == fccXX_pack_type::fcc12_packed )
        {
            constexpr bool is_mipi = pack_type == fccXX_pack_type::fcc12_mipi;
            for( int x = 0, src_offset = 0; x < (width - 16); x += 16, src_offset += 24 )
            {
                transform_wb_fcc12m_or_12p_to_fcc8_neon_step_v0<is_mipi>( dst_line + x, src_line + src_offset, mul_even, mul_odd );
            }

            transform_wb_fcc12m_or_12p_to_fcc8_neon_step_v0<is_mipi>( dst_line + width - 16, src_line + ((width - 16) / 2) * 3, mul_even, mul_odd );
        }
        else if constexpr( pack_type == fccXX_pack_type::fcc10_mipi )
        {
            constexpr bool is_mipi = pack_type == fccXX_pack_type::fcc10_mipi;
            for( int x = 0, src_offset = 0; x < (width - 16); x += 16, src_offset += 20 )
            {
                transform_wb_fcc10m_to_fcc8_neon_step_v0( dst_line + x, src_line + src_offset, mul_even, mul_odd );
            }

            transform_wb_fcc10m_to_fcc8_neon_step_v0( dst_line + width - 16, src_line + ((width - 16) / 4) * 5, mul_even, mul_odd );
        }
        else
        {
            static_assert(pack_type == fccXX_pack_type::fcc12_mipi, "pack type not implemented");
        }
    }


    template<fccXX_pack_type pack_type>
    void transform_wb_fcc12m_to_fcc8_neon_v0( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params )
    {
        assert( dst.dim.cx % 2 == 0 );
        assert( dst.dim.cx >= 16 );

        img_filter::bayer_pattern_parameters wb_params{ src.fourcc_type(), params };

        uint8x8_t wb_params_x0y0 = vdup_n_u8( static_cast<uint8_t>(wb_params.wb_x0y0 * (64)) );
        uint8x8_t wb_params_x1y0 = vdup_n_u8( static_cast<uint8_t>(wb_params.wb_x1y0 * (64)) );
        uint8x8_t wb_params_x0y1 = vdup_n_u8( static_cast<uint8_t>(wb_params.wb_x0y1 * (64)) );
        uint8x8_t wb_params_x1y1 = vdup_n_u8( static_cast<uint8_t>(wb_params.wb_x1y1 * (64)) );

        for( int y = 0; y < src.dim.cy; y += 2 )
        {
            auto* src_line0 = img::get_line_start<const uint8_t>( src, y + 0 );
            auto* dst_line0 = img::get_line_start<uint8_t>( dst, y + 0 );

            transform_wb_fcc1x_to_fcc8_neon_line_v0<pack_type>( dst_line0, src_line0, dst.dim.cx, wb_params_x0y0, wb_params_x1y0 );

            auto* src_line1 = img::get_line_start<const uint8_t>( src, y + 1 );
            auto* dst_line1 = img::get_line_start<uint8_t>( dst, y + 1 );
            transform_wb_fcc1x_to_fcc8_neon_line_v0<pack_type>( dst_line1, src_line1, dst.dim.cx, wb_params_x0y1, wb_params_x1y1 );
        }
    }

    void transform_wb_fcc1x_to_fcc8_neon_sep( const img::img_descriptor& dst, const img::img_descriptor& src, img_filter::filter_params& params )
    {
        auto func = img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_neon_v0( dst.to_img_type(), src.to_img_type() );
        assert( func );
        func( dst, src );
        auto wb_func = img_filter::whitebalance::get_apply_img_neon( dst.to_img_type() );
        wb_func( dst, params.whitebalance );
    }
}


auto img_filter::transform::fcc1x_packed::get_transform_fcc1x_to_fcc8_neon_v0( const img::img_type& dst, const img::img_type& src ) -> transform_function_param_type
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
    case fccXX_pack_type::fcc12:            return ::transform_wb_fcc12m_to_fcc8_neon_v0<fccXX_pack_type::fcc12>;
    case fccXX_pack_type::fcc12_mipi:       return ::transform_wb_fcc12m_to_fcc8_neon_v0<fccXX_pack_type::fcc12_mipi>;
    case fccXX_pack_type::fcc12_packed:     return ::transform_wb_fcc12m_to_fcc8_neon_v0<fccXX_pack_type::fcc12_packed>;
    case fccXX_pack_type::fcc12_spacked:    return transform_wb_fcc1x_to_fcc8_neon_sep;

    case fccXX_pack_type::fcc10:            return ::transform_wb_fcc12m_to_fcc8_neon_v0<fccXX_pack_type::fcc10>;
    case fccXX_pack_type::fcc10_spacked:    return transform_wb_fcc1x_to_fcc8_neon_sep;
    case fccXX_pack_type::fcc10_mipi:       return transform_wb_fcc1x_to_fcc8_neon_sep;
    case fccXX_pack_type::invalid:          return nullptr;
    };
    return nullptr;
}

auto img_filter::transform::fcc1x_packed::get_transform_fcc1x_to_fcc8_neon_sep( const img::img_type& dst, const img::img_type& src ) -> transform_function_param_type
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
    case fccXX_pack_type::fcc12:        
    case fccXX_pack_type::fcc12_mipi:   
    case fccXX_pack_type::fcc12_packed: 
    case fccXX_pack_type::fcc12_spacked:

    case fccXX_pack_type::fcc10:        
    case fccXX_pack_type::fcc10_spacked:
    case fccXX_pack_type::fcc10_mipi:
        return transform_wb_fcc1x_to_fcc8_neon_sep;

    case fccXX_pack_type::invalid:          return nullptr;
    };
    return nullptr;
}