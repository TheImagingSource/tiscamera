

#pragma once

#include "../../dutils_img_base.h"

namespace img_filter::whitebalance
{
    using apply_params = img_filter::whitebalance_params;

    /* Applies the passed white-balance values to either the image itself or the destination image.
     * white balance values must be in the range [0,0xFF] with 64 being the neutral value (* 1).
     * The functions do not check for availability of SSE2.
     * Currently SSE2 is implemented with unaligned reads/stores and may be slow on CPUs which
     * run faster with aligned stores/reads. (Core Duo/Core Quad)
     */
    namespace detail
    {
        void		apply_wb_by8_c( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb );
        void		apply_wb_by8_sse2( const img::img_descriptor& data, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb );
        void		apply_wb_by8_neon( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb );

        void		apply_wb_by16_c( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb );
        void		apply_wb_by16_sse4_1( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb );
        void		apply_wb_by16_neon( const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb );

        void		apply_wb_byfloat_c( const img::img_descriptor& dst, const apply_params& params );
    }

    using old_func_type = void (*)(const img::img_descriptor& dst, uint8_t wb_r, uint8_t wb_gr, uint8_t wb_b, uint8_t wb_gb);

    template<old_func_type actual_apply_func>
    void wrap_apply_func_to_u8( const img::img_descriptor& dst, const apply_params& params )
    {
        const uint8_t r = (uint8_t)CLIP( params.wb_rr * 64.0f, 0.f, 255.0f );
        const uint8_t b = (uint8_t)CLIP( params.wb_bb * 64.0f, 0.f, 255.0f );
        const uint8_t gr = (uint8_t)CLIP( params.wb_gr * 64.0f, 0.f, 255.0f );
        const uint8_t gb = (uint8_t)CLIP( params.wb_gb * 64.0f, 0.f, 255.0f );

        actual_apply_func( dst, r, gr, b, gb );
    }

    using func_type = void(*)( const img::img_descriptor& dst, const apply_params& params );

    func_type  get_apply_img_c( img::img_type dst );
    func_type  get_apply_img_sse41( img::img_type dst );
    func_type  get_apply_img_neon( img::img_type dst );
}

