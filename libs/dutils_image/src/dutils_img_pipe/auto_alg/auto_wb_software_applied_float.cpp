
#include "auto_alg.h"

#include <algorithm>
#include "../../dutils_img_base/interop_private.h"

namespace 
{
    const constexpr float WB_IDENTITY = 1.f;
    const constexpr float WB_MAX = 4.f;
    const constexpr int MAX_STEPS = 40;
    const constexpr float WB_STEPSIZE = 0.001f;

    const constexpr float BREAK_DIFF = 0.001;//2 / 255.f;

    const float NEARGRAY_MAX_COLOR_DEVIATION = 0.25f;
    const float NEARGRAY_REQUIRED_AMOUNT = 0.08f;   // => at least 8% must be near gray to use the neargray stuff


    using std::abs;

    struct wb_components : auto_alg::impl::RGBf
    {
        wb_components&   operator+=( const RGBf rhs ) noexcept {
            r += rhs.r;
            g += rhs.g;
            b += rhs.b;
            return *this;
        }
    };

    static wb_components    operator/( auto_alg::impl::RGBf val, int div ) noexcept
    {
        return wb_components{ val.r / div, val.g / div, val.b / div };
    }
    static wb_components    operator-( auto_alg::impl::RGBf val, float rhs ) noexcept
    {
        return wb_components{ val.r - rhs, val.g - rhs, val.b - rhs };
    }

    constexpr float clip_to_wb_range( float f ) noexcept {
        return CLIP( f, 0.f, WB_MAX );
    }
    constexpr float clip_to_pixel_range( float f ) noexcept {
        return CLIP( f, 0.f, 1.f );
    }
    constexpr auto to_wb_channel_factors( wb_components values ) noexcept -> auto_alg::wb_channel_factors {
        return {
            clip_to_wb_range( values.r ),
            clip_to_wb_range( values.g ),
            clip_to_wb_range( values.b ),
        };
    }

    // This result in that at least one of r/g/b is WB_IDENTITY
    static wb_components   adjust_to_identity( wb_components wb ) noexcept
    {
        const auto dist0 = std::min( { wb.r - WB_IDENTITY, wb.b - WB_IDENTITY, wb.g - WB_IDENTITY } );
        if( dist0 > 0.f ) {
            return wb - dist0;
        }
        return wb;
    }

    inline bool is_near_gray( const auto_alg::impl::RGBf pix ) noexcept
    {
        constexpr float NEARGRAY_MIN_BRIGHTNESS = 1.0f / (1 << 16); // prev 10 / 255.f;
        constexpr float NEARGRAY_MAX_BRIGHTNESS = 0.999f; // prev 253 / 255.f;

        using std::abs;

        const float brightness = auto_alg::impl::calc_brightness_from_clr_avgf( pix );
        if( brightness < NEARGRAY_MIN_BRIGHTNESS ) return false;
        if( brightness > NEARGRAY_MAX_BRIGHTNESS ) return false;

        const float deltaR = abs( pix.r - brightness );
        const float deltaG = abs( pix.g - brightness );
        const float deltaB = abs( pix.b - brightness );

        const float devR = deltaR / brightness;
        const float devG = deltaG / brightness;
        const float devB = deltaB / brightness;

        return (devR < NEARGRAY_MAX_COLOR_DEVIATION) && (devG < NEARGRAY_MAX_COLOR_DEVIATION) && (devB < NEARGRAY_MAX_COLOR_DEVIATION);
    }

static float calc_step( float /*delta*/ ) noexcept
{
    return WB_STEPSIZE;
}

static bool wb_auto_step( const wb_components& clr, wb_components& wb ) noexcept
{
    // first normalize components such that the average is always 1.0f
    const float avg = ((clr.r + clr.g + clr.b) / 3);
    const float normalization_factor = 1.0f / avg;
    wb_components normalized_color = { clr.r * normalization_factor , clr.g * normalization_factor, clr.b * normalization_factor };

    const float dr = avg - normalized_color.r;
    const float dg = avg - normalized_color.g;
    const float db = avg - normalized_color.b;

    if( abs( dr ) < BREAK_DIFF && abs( dg ) < BREAK_DIFF && abs( db ) < BREAK_DIFF )
    {
        return true;
    }

    if( (clr.r > avg) && (wb.r > WB_IDENTITY) )
    {
        wb.r -= calc_step( dr );
    }
    else if( (clr.r < avg) && (wb.r < WB_MAX) )
    {
        wb.r += calc_step( dr );
    }

    if( (clr.g > avg) && (wb.g > WB_IDENTITY) )
    {
        wb.g -= calc_step( dg );
    }
    else if( (clr.g < avg) && (wb.g < WB_MAX) )
    {
        wb.g += calc_step( dg );
    }

    if( (clr.b > avg) && (wb.b > WB_IDENTITY) )
    {
        wb.b -= calc_step( db );
    }
    else if( (clr.b < avg) && (wb.b < WB_MAX) )
    {
        wb.b += calc_step( db );
    }

    wb = adjust_to_identity( wb );
    return false;
}

static wb_components simulate_whitebalance( const auto_alg::impl::image_sampling_points_rgbf& data, const wb_components& wb ) noexcept
{
    wb_components result = { 0.f, 0.f, 0.f };
    wb_components result_near_gray = { 0.f, 0.f, 0.f };
    
    int count_near_gray = 0;
    for( int i = 0; i < data.cnt; ++i )
    {
        const auto src_pix = data.samples[i];

        auto_alg::impl::RGBf pix = {
            clip_to_pixel_range( src_pix.r * wb.r ),
            clip_to_pixel_range( src_pix.g * wb.g ),
            clip_to_pixel_range( src_pix.b * wb.b ),
        };

        result += pix;

        if( is_near_gray( pix ) )
        {
            result_near_gray += pix;
            count_near_gray += 1;
        }
    }

    const float near_gray_amount = count_near_gray / (float)data.cnt;
    if( near_gray_amount < NEARGRAY_REQUIRED_AMOUNT )
    {
        return result / data.cnt;
    }
    else
    {
        return result_near_gray / count_near_gray;
    }
}

}


auto auto_alg::impl::auto_whitebalance_soft( const auto_alg::impl::image_sampling_points_rgbf& data, 
    const wb_channel_factors& wb_in_factors ) -> auto_whitebalance_result
{
    wb_components wb = { wb_in_factors.r, wb_in_factors.g, wb_in_factors.b };

    // for values < identity set them to identity
    if( wb.r < WB_IDENTITY ) wb.r = WB_IDENTITY;
    if( wb.g < WB_IDENTITY ) wb.g = WB_IDENTITY;
    if( wb.b < WB_IDENTITY ) wb.b = WB_IDENTITY;

    wb = adjust_to_identity( wb );

    assert( data.cnt > 0 );
    
    for( int steps = 0; steps < MAX_STEPS; ++steps )
    {
        wb_components tmp = simulate_whitebalance( data, wb );
        if( wb_auto_step( tmp, wb ) ) {
			return { true, to_wb_channel_factors( wb ) };
        }
    }

	return { false, to_wb_channel_factors( wb ) };
}
