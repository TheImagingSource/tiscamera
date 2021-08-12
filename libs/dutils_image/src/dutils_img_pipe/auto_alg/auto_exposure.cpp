

#include "auto_exposure.h"

#include <cmath>
#include <algorithm>
#include <cstdio>

#include "../../dutils_img_base/interop_private.h"

namespace
{
    using std::abs;

static float calc_dist( float reference_val, float brightness_ref ) noexcept
{
    auto div = CLIP( brightness_ref, 0.004f, 1.f );
    return reference_val / div;
}

static unsigned int calc_exposure( float dist, int exposure, const auto_alg::property_cont_exposure& range )
{
    dist = ( dist + 2.f ) / 3;
    exposure = static_cast<int>( exposure * dist );

    // If we do not do a significant change (on the sensor-scale), don't change anything
    // This should avoid pumping caused by an abrupt brightness change caused by a small value change
    if( abs(exposure - range.val) < (range.granularity/2) ) {
        return range.val;
    }

    return CLIP( exposure, range.min, range.max );
}

static float calc_gain_db( float dist, float gain, const auto_alg::property_cont_gain& range )
{
    if( dist >= 1.f ) {	// when we have to reduce, we reduce it faster
        dist = (dist + 2.f) / 3.f;	// this dampens the change in dist factor
    }

    float val = log2f( dist ) * range.gain_db_multiplier;
    gain += val;

    return CLIP( gain, range.min, range.max );
}

// This gets used for DFK 72 and stuff where gain is used as a multiplier
static float calc_gain_multiplier( float dist, float gain, const auto_alg::property_cont_gain& range )
{
    if( dist >= 1.f ) {	// when we have to reduce, we reduce it faster
        dist = (dist + 2.f) / 3.f;	// this dampens the change in dist factor
    }

    if( gain == 0 ) {
        gain = 1;
    }
    gain = gain * dist;

    return CLIP( gain, range.min, range.max );
}

static float calc_gain( float dist, float gain, const auto_alg::property_cont_gain& range )
{
    if( range.is_gain_db ) {
        return calc_gain_db( dist, gain, range );
    } else {
        return calc_gain_multiplier( dist, gain, range );
    }
}

static int calc_iris( float dist, int iris, const auto_alg::property_cont_iris& range )
{
    dist = (dist + 3.f) / 4.f;	// this dampens the change in dist factor

    if( iris == 0 ) {
        iris = 1;
    }
    iris = static_cast<int>(iris * dist);

    return CLIP( iris, range.min, range.max );
}

}

auto_alg::impl::gain_exposure_iris_values		auto_alg::impl::calc_auto_gain_exposure_iris( 
    float brightness, float reference_value, 
    const auto_alg::property_cont_gain& gain_desc,
    const auto_alg::property_cont_exposure& exposure_desc, 
    const auto_alg::property_cont_iris& iris_desc )
{
    auto_alg::impl::gain_exposure_iris_values rval = {};
    if( exposure_desc.auto_enabled ) {
        rval.exposure = CLIP( exposure_desc.val, exposure_desc.min, exposure_desc.max );
    } else {
        rval.exposure = exposure_desc.val;
    }

    if( gain_desc.auto_enabled ) {
        rval.gain = CLIP( gain_desc.value, gain_desc.min, gain_desc.max );
    } else {
        rval.gain = gain_desc.value;
    }

    if( iris_desc.auto_enabled ) {
        rval.iris = CLIP( iris_desc.val, iris_desc.min, iris_desc.max );
    } else {
        rval.iris = iris_desc.val;
    }

    float gain = rval.gain;
    int exposure = rval.exposure;
    int iris = iris_desc.val;;

    float dist = calc_dist( reference_value, brightness );
    if( dist < 0.96f || dist > 1.04f )
    {
        float tmp_gain = 0;
        if( gain_desc.auto_enabled )
        {
            // reduce gain, if possible
            tmp_gain = calc_gain( dist, gain, gain_desc );
            if( tmp_gain < gain )
            {
                rval.gain = tmp_gain;
                return rval;
            }
        }

        int tmp_iris = 0;
        if( iris_desc.auto_enabled )
        {
            // iris
            tmp_iris = calc_iris( dist, iris, iris_desc );
            if( tmp_iris < iris )
            {
                rval.iris = tmp_iris;
                return rval;
            }
        }

        int tmp_exposure = 0;
        if( exposure_desc.auto_enabled )
        {
            // exposure
            tmp_exposure = calc_exposure( dist, exposure, exposure_desc );
            if( tmp_exposure != exposure )
            {
                rval.exposure = tmp_exposure;
                return rval;
            }
        }

        if( iris_desc.auto_enabled )
        {
            // when gain is in a sweet spot, or cannot be increased anymore
            if( tmp_iris != iris /*&& exposure >= exposure_desc.max*/ )
            {
                rval.iris = tmp_iris;
                return rval;
            }
        }

        if( gain_desc.auto_enabled )
        {
            // when exposure is in a sweet spot, or cannot be increased anymore
            if( tmp_gain != gain /*&& exposure >= exposure_desc.max*/ )
            {
                rval.gain = tmp_gain;
                return rval;
            }
        }
    }

    if( exposure_desc.auto_enabled && gain_desc.auto_enabled &&
        gain > gain_desc.min && exposure < exposure_desc.max			// we can reduce gain, because we can increase exposure
        )
    {
        rval.exposure = CLIP( (exposure * 105) / 100, exposure_desc.min, exposure_desc.max );	// increase exposure by 5%
        return rval;
    }

    if( exposure_desc.auto_enabled && iris_desc.auto_enabled &&
        iris > iris_desc.min && exposure < exposure_desc.max			// we can reduce iris, because we can increase exposure
        )
    {
        rval.exposure = CLIP( (exposure * 105) / 100, exposure_desc.min, exposure_desc.max );	// increase exposure by 5%
        return rval;
    }


    if( gain_desc.auto_enabled &&  iris_desc.auto_enabled &&
        gain > gain_desc.min && iris < iris_desc.max			// we can reduce gain, because we can increase iris
        )
    {
        rval.iris = CLIP( (iris * 105) / 100, iris_desc.min, iris_desc.max );	// increase iris by 5%
        return rval;
    }
    return rval;
}

int auto_alg::impl::calc_auto_pwm_iris( float corrected_brightness, int reference_value, const auto_alg::property_cont_iris& iris_desc, detail::pid_controller& iris_controller )
{
    // Don't calculate with too slow frame rates, the iris might go too fast
    const float fps_ceil = std::min( static_cast<float>( iris_desc.camera_fps ), 60.f );

    const float e = reference_value - corrected_brightness;
    const float y = iris_controller.step( e, fps_ceil );

    const int iris_mid = (iris_desc.max - iris_desc.min) / 2;
    const int iris_val = (int)(iris_mid - y);

    return CLIP(iris_val, iris_desc.min, iris_desc.max);		
}
