

#include "empia_pattern_fix.h"

#include "auto_alg.h"
#include <algorithm>
#include "image_sampling_u8.h"

namespace
{

static img_filter::whitebalance::apply_params  find_average( const auto_alg::impl::auto_sample_points& points ) noexcept
{
    int rr = 0, gr = 0, bb = 0, gb = 0;

    for( int i = 0; i < points.cnt; ++i )
    {
        rr += points.samples[i].rr;
        gr += points.samples[i].gr;
        bb += points.samples[i].bb;
        gb += points.samples[i].gb;
    }

    return {
        true,
        ((float)rr / points.cnt / 255.f),
        ((float)gr / points.cnt / 255.f),
        ((float)bb / points.cnt / 255.f),
        ((float)gb / points.cnt / 255.f)
    };
}

}

static img_filter::whitebalance::apply_params        calc_empia_wb_values( const img::img_descriptor& src, auto_alg::impl::auto_sample_points& points )
{
    img::img_descriptor by16src = img::replace_fcc( src, img::fourcc::RGGB8 );

    auto_alg::impl::auto_sample_by_imgu8( by16src, points );
    if( points.cnt == 0 ) {
        return {};
    }

    const auto avg = find_average( points );
    if( avg.wb_rr == 0 || avg.wb_gr == 0 || avg.wb_gb == 0 || avg.wb_bb == 0 ) {
        return {};
    }

    const float target = std::max( { avg.wb_rr, avg.wb_gr, avg.wb_bb, avg.wb_gb } );

    return img_filter::whitebalance::apply_params{
        true,
        target / avg.wb_rr,
        target / avg.wb_gr,
        target / avg.wb_bb,
        target / avg.wb_gb,
    };
}


img_filter::whitebalance::apply_params  img_filter::empia_fix::calc_empia_wb_values( const img::img_descriptor& src, auto_alg::impl::auto_sample_points& scratch_space )
{
    if( src.fourcc_type() == img::fourcc::MONO16 ) {
        return ::calc_empia_wb_values( img::replace_fcc( src, img::fourcc::RGGB16 ), scratch_space );
    }
    if( src.fourcc_type() == img::fourcc::MONO8 ) {
        return ::calc_empia_wb_values( img::replace_fcc( src, img::fourcc::RGGB8 ), scratch_space );
    }
    assert( false );
    return {};
}
