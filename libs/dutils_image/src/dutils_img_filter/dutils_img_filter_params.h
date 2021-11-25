
#pragma once

#include <dutils_img/dutils_img.h>
#include <dutils_img/image_bayer_pattern.h>
#include "../dutils_img_base/interop_private.h"
#include <dutils_img/image_transform_data_structs.h>

namespace img_filter
{
    using img::whitebalance_params;

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
        static bayer_pattern_parameters     convert( img::by_transform::by_pattern pat, const img::whitebalance_params& params ) noexcept;

        bayer_pattern_parameters() = default;
        constexpr bayer_pattern_parameters( img::by_transform::by_pattern pat, const img::whitebalance_params& params ) noexcept
        {
            using namespace img::by_transform;
            switch( pat )
            {
            case by_pattern::BG: wb_x0y0 = params.wb_bb; wb_x1y0 = params.wb_gb; wb_x0y1 = params.wb_gr; wb_x1y1 = params.wb_rr; break;
            case by_pattern::GB: wb_x0y0 = params.wb_gb; wb_x1y0 = params.wb_bb; wb_x0y1 = params.wb_rr; wb_x1y1 = params.wb_gr; break;
            case by_pattern::GR: wb_x0y0 = params.wb_gr; wb_x1y0 = params.wb_rr; wb_x0y1 = params.wb_bb; wb_x1y1 = params.wb_gb; break;
            case by_pattern::RG: wb_x0y0 = params.wb_rr; wb_x1y0 = params.wb_gr; wb_x0y1 = params.wb_gb; wb_x1y1 = params.wb_bb; break;
            }
        }
        constexpr bayer_pattern_parameters( img::fourcc fcc, const filter_params& params ) noexcept
            : bayer_pattern_parameters( img::by_transform::convert_bayer_fcc_to_pattern( fcc ), params.whitebalance ) {}


        float wb_x0y0 = 0;
        float wb_x1y0 = 0;
        float wb_x0y1 = 0;
        float wb_x1y1 = 0;
    };

    inline     bayer_pattern_parameters     bayer_pattern_parameters::convert( img::by_transform::by_pattern pat, const img::whitebalance_params& params ) noexcept
    {
        return bayer_pattern_parameters( pat, params );
    }
}

