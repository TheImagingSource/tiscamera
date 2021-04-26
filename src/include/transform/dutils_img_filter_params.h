
#pragma once

//#include <dutils_img/dutils_img.h>
#include <img/image_bayer_pattern.h>
#include "img/interop_private.h"

namespace img_filter
{
    struct whitebalance_params
    {
        bool    apply = false;
        float   wb_rr = 1.f;
        float   wb_gr = 1.f;
        float   wb_bb = 1.f;
        float   wb_gb = 1.f;
    };

    /** Transforms the struct to either the default or clips the elements to the whitebalance range
     * If !apply then all members will be set to 1.f
     * Else all elements are clipped to the range of [0.f;4.f]
     */
    constexpr whitebalance_params normalize( const whitebalance_params& param ) noexcept {
        if( !param.apply ) {
            return whitebalance_params{};
        }
        whitebalance_params tmp{ true };
        tmp.wb_rr = CLIP( param.wb_rr, 0.f, 4.f );
        tmp.wb_gr = CLIP( param.wb_gr, 0.f, 4.f );
        tmp.wb_bb = CLIP( param.wb_bb, 0.f, 4.f );
        tmp.wb_gb = CLIP( param.wb_gb, 0.f, 4.f );
        return tmp;
    }

    struct pwl12_to_fcc8_wb_map_data;

    struct filter_params
    {
        whitebalance_params             whitebalance;
        pwl12_to_fcc8_wb_map_data*      pwl12_to_fcc8_wb_lut = nullptr;
    };

    struct bayer_pattern_parameters
    {
        static bayer_pattern_parameters     convert( img::by_transform::by_pattern pat, const img_filter::whitebalance_params& params ) noexcept;

        float wb_x0y0 = 0;
        float wb_x1y0 = 0;
        float wb_x0y1 = 0;
        float wb_x1y1 = 0;
    };

    inline     bayer_pattern_parameters     bayer_pattern_parameters::convert( img::by_transform::by_pattern pat, const img_filter::whitebalance_params& params ) noexcept
    {
        switch( pat )
        {
        case img::by_transform::by_pattern::BG: return { params.wb_bb, params.wb_gb, params.wb_gr, params.wb_rr };
        case img::by_transform::by_pattern::GB: return { params.wb_gb, params.wb_bb, params.wb_rr, params.wb_gr };
        case img::by_transform::by_pattern::GR: return { params.wb_gr, params.wb_rr, params.wb_bb, params.wb_gb };
        case img::by_transform::by_pattern::RG: return { params.wb_rr, params.wb_gr, params.wb_gb, params.wb_bb };
        }
        return { params.wb_rr, params.wb_gr, params.wb_gb, params.wb_bb };
    }
}

