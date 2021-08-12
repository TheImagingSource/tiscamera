
#pragma once

#include <cmath>
#include "auto_sample_image.h"

namespace auto_alg::impl::wb
{
	const int NEARGRAY_MIN_BRIGHTNESS = 10;
	const int NEARGRAY_MAX_BRIGHTNESS = 253;
	const float NEARGRAY_MAX_COLOR_DEVIATION = 0.25f;
	const float NEARGRAY_REQUIRED_AMOUNT = 0.08f;

	const int MAX_STEPS = 20;
	const int WB_IDENTITY = 64;
	const int WB_MAX = 255;
	const int BREAK_DIFF = 2;

	inline int clip_to_wb( int x )
	{
		if( x > WB_MAX )	return WB_MAX;
		if( x < 0 )			return 0;
		return x;
	}

	struct rgb_tripel
	{
		int r, g, b;

		void operator+=( const auto_alg::impl::pixel& pix ) noexcept {
			r += pix.r;
            g += pix.g;
            b += pix.b;
		}
        void operator/=( int scale ) noexcept {
            r /= scale;
            g /= scale;
            b /= scale;
        }
	};


	constexpr auto to_wb_channel_factors( rgb_tripel tripel ) -> wb_channel_factors {
		return { tripel.r / 64.f, tripel.g / 64.f, tripel.b / 64.f };
	}
	constexpr auto to_rgb_tripel( wb_channel_factors tripel ) -> rgb_tripel {
		return { (int)(tripel.r * 64.f), (int)(tripel.g * 64.f), (int)(tripel.b * 64.f) };
	}

    inline bool is_near_gray( const auto_alg::impl::pixel pix ) noexcept
    {
		using std::abs;

        const int brightness = auto_alg::impl::calc_brightness_from_clr_avg( pix );
        if( brightness < NEARGRAY_MIN_BRIGHTNESS ) return false;
        if( brightness > NEARGRAY_MAX_BRIGHTNESS ) return false;

        const int deltaR = abs( int( pix.r ) - brightness );
        const int deltaG = abs( int( pix.g ) - brightness );
        const int deltaB = abs( int( pix.b ) - brightness );

        const float devR = deltaR / (float)brightness;
        const float devG = deltaG / (float)brightness;
        const float devB = deltaB / (float)brightness;

        return (devR < NEARGRAY_MAX_COLOR_DEVIATION) && (devG < NEARGRAY_MAX_COLOR_DEVIATION) && (devB < NEARGRAY_MAX_COLOR_DEVIATION);
    }
}