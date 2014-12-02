
#include "../image_transform_interop.h"

#include "auto_alg_pass.h"

#include "auto_alg.h"

#include <math.h>
#include <stdlib.h>
#include "auto_focus.h"

#include "pid_controller.h"
#include "../img/img_desc_tools.h"

using namespace auto_alg;

namespace auto_alg
{
    struct pwm_iris_pid_controller
    {
        auto_alg::pid_controller	iris_controller;

        pwm_iris_pid_controller();
    };

    struct auto_pass_state
    {
        int         auto_apply_distance;
        int         auto_apply_count;

        pwm_iris_pid_controller	pwm_iris;

        struct {
            int		current_reference;
            int		calculated_reference;
        } auto_ref;

        struct {
            int		one_push_step_count;
        } wb_temperature;

        bool        is_software_applied_wb;
        bool        use_focus_one_push;

        img::auto_focus     focus_onepush_provider;

        auto_sample_points  points;
        auto_sample_points  points_for_wb_temperature;
    };
}


auto_alg::pwm_iris_pid_controller::pwm_iris_pid_controller()
: iris_controller( 0.4f, 2.0f, 1.0f, 4000 )
{

}


auto_alg::pid_controller::pid_controller( float p, float i, float d, float e_sum_limit )
: _P( p ),
_I( i ),
_D( d ),
_e_sum_limit( e_sum_limit ),
_e_sum( 0 ),
_e_prev( 0 ),
_e_prev_valid( false )
{
}

void	auto_alg::pid_controller::reset()
{
    _e_sum = 0;
    _e_prev_valid = false;
}

float	auto_alg::pid_controller::step( float e, float fps )
{
	_e_sum += e;

	if( fps == 0.0f )
		fps = 1.0f;

	float p = _P * e;
	float i = _I * _e_sum / fps;
	float d = 0;

	if( _e_prev_valid )
		d = _D * (e - _e_prev) / fps;

	if( _e_sum > _e_sum_limit ) _e_sum = _e_sum_limit;
	if( _e_sum < -_e_sum_limit ) _e_sum = -_e_sum_limit;

	return p + i + d;
}


static float calc_possible_automatic_reduction( const property_cont_exposure& exposure, int exp_val )
{
	return exp_val / (float)exposure.min;
}

static float calc_possible_automatic_reduction( const property_cont_gain& gain, int gain_val )
{
#if !defined KERNEL_DRIVER_
	if( gain.is_db_gain )
	{
		return exp( gain_val / 301.0f * log( 2.0f ) );
	}
	else if( gain.max == 1023 )
	{
		// Detect CCD cameras, use given steps-double value
		return powf( 2.0f, (float)((gain_val - gain.min) / gain.steps_to_double_brightness) );
	}
	else
#endif
	{
		// Detect CMOS cameras: Expect gain to be multiplier
		return (gain_val / (float)gain.min);
	}
}

static auto_alg::gain_exposure_iris_values		auto_alg_for_brightness_adjust( const auto_pass_params& exp, int brightness, int reference, pwm_iris_pid_controller& state )
{
	property_cont iris_tmp = exp.iris;
	if( exp.iris.is_pwm_iris )
	{
		iris_tmp.do_auto = false;
	}

	gain_exposure_iris_values res = auto_alg::calc_auto_gain_exposure_iris( brightness, reference, exp.gain, exp.exposure, iris_tmp );

	if( exp.iris.is_pwm_iris && exp.iris.do_auto )
	{
		float corrected_brightness = (float)brightness;

		// If gain and/or exposure are on auto, reduce the brightness according to how much lower they could go
		if( exp.gain.do_auto )
		{
			corrected_brightness /= calc_possible_automatic_reduction( exp.gain, res.gain );
		}
		if( exp.exposure.do_auto )
		{
			corrected_brightness /= calc_possible_automatic_reduction( exp.exposure, res.exposure );
		}
		res.iris = calc_auto_pwm_iris( corrected_brightness, exp.exposure_reference.val, exp.iris, state.iris_controller );
	}
	return res;
}


static int calc_adjusted_auto_reference_( float factor_of_y_values_greater_240, int user_reference, int calculated_reference )
{
	int reference_value = calculated_reference;
	// decrease reference if to much high tones
	if( factor_of_y_values_greater_240 > 0.15f )
	{
		reference_value -= (int)MIN( 8, (40.0f * factor_of_y_values_greater_240 - 4.0f + 0.5f) );
	}
	// increase reference, if reference less than 128 and not much high tones
	else if( reference_value < user_reference && factor_of_y_values_greater_240 < 0.01f )
	{
		reference_value += (int)(400.0f * factor_of_y_values_greater_240 + 2.0f);
	}
	return CLIP( reference_value, 0, 255 );
}

static int calc_adjusted_auto_reference( float factor_of_y_values_greater_240, int user_reference, int applied_reference, int& calculated_reference )
{
	calculated_reference = calc_adjusted_auto_reference_( factor_of_y_values_greater_240, user_reference, calculated_reference );
	// only significant changes
	if( abs( applied_reference - calculated_reference ) > 8 )
	{
		return calculated_reference;
	}
	return applied_reference;
}

struct auto_wb_rval {
	auto_alg::rgb_tripel	rgb;
	int						current_temperature;
	bool					one_push_still_running;
};

static auto_wb_rval update_whitebalance( const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr, const whitebalance_values& wb, auto_pass_state& state )
{
	auto_wb_rval rval = { { wb.r, wb.g, wb.b }, wb.temperature.current, wb.one_push_enabled };

	bool do_auto = wb.auto_enabled || wb.one_push_enabled;
	if( !do_auto )
		return rval;

	bool keep_onepush_running = false;
	if( wb.temperature_mode && wb.temperature.temperature_to_factors_arr != NULL )
	{
        // this construct makes that we can use the already passed in struct for software applied wb devices and only need to make a copy of the array
        // for hardware applied wb devices
        // const auto_alg::auto_sample_points* p_tmp_points = NULL;
        // if( !wb.is_software_applied_wb )
        // {
        //     for( int i = 0; i < points.cnt; ++i )
		//     {
		// 	    auto_sample_points::pixel tripel = points.samples[i];

        //         // we have to scale the points back to the not-white-balanced source to get the right values
        //         // software applied wb is applied later to the image, so we don't need to revert the image to the 'pre-'-white-balanced source
		// 		tripel.r = (byte)CLIP( ((float)tripel.r * 64.0f) / (float)wb.r, 0, 0xFF );
		// 		tripel.g = (byte)CLIP( ((float)tripel.g * 64.0f) / (float)wb.g, 0, 0xFF );
		// 		tripel.b = (byte)CLIP( ((float)tripel.b * 64.0f) / (float)wb.b, 0, 0xFF );

        //         state.points_for_wb_temperature.samples[i] = tripel;
		//     }
        //     state.points_for_wb_temperature.cnt = points.cnt;

        //     p_tmp_points = &state.points_for_wb_temperature;
        // }
        // else
        // {
        //     p_tmp_points = &points;
        // }
        // // we reference the points array here, because it may be too large for small stacks
        // const auto_alg::auto_sample_points& tmp = *p_tmp_points;

		// int new_temperature;
		// if( wb.one_push_enabled )
		// {
		// 	new_temperature = calc_temperature_for_pixels( tmp, wb.temperature.auto_min, wb.temperature.auto_max, wb.temperature.temperature_to_factors_arr );
		// 	if( new_temperature == -1 )
		// 		new_temperature = wb.temperature.current;
		// 	if( --state.wb_temperature.one_push_step_count <= 0 )
		// 	{
		// 		state.wb_temperature.one_push_step_count = 5;
		// 		keep_onepush_running = false;
		// 	}
		// 	else
		// 	{
		// 		keep_onepush_running = true;
		// 	}
		// }
		// else
		// {
		// 	new_temperature = calc_temperature_auto_step( tmp, wb.temperature.current, wb.temperature.auto_min, wb.temperature.auto_max, wb.temperature.temperature_to_factors_arr );
		// 	keep_onepush_running = false;
		// }

		// rval.rgb = auto_alg::calc_whitebalance_values( new_temperature, wb.temperature.temperature_to_factors_arr );
		// rval.current_temperature = new_temperature;
	}
	else
	{
		if( !wb.is_software_applied_wb )
		{
			keep_onepush_running = !auto_alg::auto_whitebalance_cam( points, clr, rval.rgb );
		}
		else
		{
			keep_onepush_running = !auto_alg::auto_whitebalance( points, clr, rval.rgb );
		}
	}
	if( wb.one_push_enabled )
		rval.one_push_still_running = keep_onepush_running;

	return rval;
}

static int		calc_auto_reference( const auto_pass_params& params, auto_pass_state& state, float factor_y_vgt240 )
{
	int reference_value_used = params.exposure_reference.val;
	if( factor_y_vgt240 >= 0.0f && params.enable_highlight_reduction && (params.exposure.do_auto || params.gain.do_auto || params.iris.do_auto) )
	{
		state.auto_ref.current_reference = calc_adjusted_auto_reference( factor_y_vgt240, params.exposure_reference.val,
																		  state.auto_ref.current_reference, state.auto_ref.calculated_reference );
		reference_value_used = state.auto_ref.current_reference * params.exposure_reference.val / 128;
	}
	else
	{
		state.auto_ref.current_reference = params.exposure_reference.val;
	}
	return reference_value_used;
}


static void exec_whitebalance_auto( const img::img_descriptor& data, const auto_pass_params& params, auto_pass_state& state, auto_pass_results& rval, float& factor_y_vgt240 )
{
    // we use the buffer in state to avoid having a very large structure on the stack
    auto_alg::auto_sample_points& points = state.points;

    auto_alg::auto_sample_by_img( data, points );

    auto_wb_rval wb_results;
    if( params.wb.auto_enabled || params.wb.one_push_enabled || params.wb.is_software_applied_wb )
    {
        wb_results = update_whitebalance( points, params.clr, params.wb, state );

        if( params.wb.auto_enabled || params.wb.one_push_enabled )
        {
            rval.wb_r = wb_results.rgb.r;
            rval.wb_g = wb_results.rgb.g;
            rval.wb_b = wb_results.rgb.b;
            rval.wb_temperature = wb_results.current_temperature;
            rval.wb_one_push_still_running = wb_results.one_push_still_running;
        }
    }

    RECT brightness_roi = img::clip_to_img_desc_region( params.brightness_roi, params.sensor_offset, params.pixel_dim, data );

    if( !img::is_empty_rect( brightness_roi ) )
    {
        img::img_descriptor img_tmp = data;
        img_tmp = img::make_img_view( data, brightness_roi );
        auto_alg::auto_sample_by_img( img_tmp, points );

        img::fill_image( img_tmp, 0xFF );
    }

    if( params.wb.is_software_applied_wb )
    {
        calc_resulting_brightness_params( rval.brightness, factor_y_vgt240, points, params.clr, wb_results.rgb );
    }
    else
    {
        calc_resulting_brightness_params( rval.brightness, factor_y_vgt240, points, params.clr );
    }
}

auto_pass_results	auto_alg::auto_pass( const img::img_descriptor& data, const auto_pass_params& params, auto_pass_context p_context )
{
    ASSERT( p_context != NULL );
    auto_pass_state& state = *p_context;

	auto_pass_results rval = {};
	rval.exposure = params.exposure.val;
	rval.gain = params.gain.val;
	rval.iris = params.iris.val;
	rval.wb_r = params.wb.r;
	rval.wb_g = params.wb.g;
	rval.wb_b = params.wb.b;
	rval.wb_one_push_still_running = false;
	rval.wb_temperature = params.wb.temperature.current;
    rval.focus_onepush_running = false;

	rval.brightness = 128;
    rval.auto_pass_has_run = false;
    rval.focus_value = params.focus_onepush_params.device_focus_val;

    if( state.use_focus_one_push != NULL )
    {
        state.focus_onepush_provider.auto_alg_run( data, params.focus_onepush_params, params.sensor_offset, params.pixel_dim, rval.focus_value );
        rval.focus_onepush_running = state.focus_onepush_provider.is_running();
    }

    if( ++state.auto_apply_count < state.auto_apply_distance )
        return rval;

    state.auto_apply_count = 0;

	float factor_y_vgt240 = -1.0f;

	switch( data.type )
	{
    case FOURCC_GRBG16:
    case FOURCC_GBRG16:
    case FOURCC_RGGB16:
    case FOURCC_BGGR16:
    case FOURCC_GRBG8:
	case FOURCC_GBRG8:
	case FOURCC_RGGB8:
	case FOURCC_BGGR8:
	case FOURCC_BY8:
        {
            exec_whitebalance_auto( data, params, state, rval, factor_y_vgt240 );
        }
        break;
	case FOURCC_Y800:
	case FOURCC_Y16:
        {
            RECT brightness_roi = img::clip_to_img_desc_region( params.brightness_roi, params.sensor_offset, params.pixel_dim, data );

            img::img_descriptor tmp_img = img::make_img_view( data, brightness_roi );
            auto_alg::auto_sample_mono_img( tmp_img, rval.brightness, factor_y_vgt240 );
        }
        break;
	default:
		break;
	};

	int reference_value_to_use = calc_auto_reference( params, state, factor_y_vgt240 );
	gain_exposure_iris_values res = auto_alg_for_brightness_adjust( params, rval.brightness, reference_value_to_use, state.pwm_iris );

	rval.exposure = res.exposure;
	rval.gain = res.gain;
	rval.iris = res.iris;

    rval.auto_pass_has_run = true;
	return rval;
}


void    auto_alg::reinit_auto_pass_context( auto_pass_context p_handle )
{
    ASSERT( p_handle != NULL );
    auto_pass_state& state = *p_handle;
    state.auto_apply_count = state.auto_apply_distance;

    state.auto_ref.calculated_reference = 128;
    state.auto_ref.current_reference = 128;
    state.wb_temperature.one_push_step_count = 0;

    state.pwm_iris.iris_controller.reset();
    state.focus_onepush_provider.reset();
}

size_t auto_alg::get_auto_pass_context_size( const create_params& /*create_params*/ )
{
    return sizeof( auto_alg::auto_pass_state );
}

auto_alg::auto_pass_context auto_alg::create_auto_pass_context( void* context_space, size_t context_space_size, const create_params& create_params )
{
    if( context_space_size < get_auto_pass_context_size( create_params ) )
        return NULL;

    auto_alg::auto_pass_context p_state = new( context_space ) auto_alg::auto_pass_state();
    if( p_state == NULL )
        return NULL;

    p_state->is_software_applied_wb = create_params.is_software_applied_wb;
    p_state->use_focus_one_push = create_params.add_software_onepush_focus;
    p_state->auto_apply_distance = create_params.auto_apply_distance;


    reinit_auto_pass_context( p_state );

    return p_state;
}


void auto_alg::destroy_auto_pass_context( auto_pass_context context )
{
    if( context != NULL )
    {
        context->~auto_pass_state();
    }
}
