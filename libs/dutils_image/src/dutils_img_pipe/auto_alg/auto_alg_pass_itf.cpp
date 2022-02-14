
#define DUTILS_IMG_PIPE_EXPORT

#include "auto_alg_pass_itf.h"

#include "auto_alg.h"
#include "auto_wb_temperature.h"
#include "auto_wb_temperature_sensor_data.h"

#include <cmath>
#include <cstdlib>
#include <memory>

#include "pid_controller.h"
#include "auto_focus.h"
#include "auto_exposure.h"

#include "../../dutils_img_base/interop_private.h"
#include "../../dutils_img_base/img_rect_tools.h"

#include <algorithm>
#include <cstring>
#include "auto_hdr_gain.h"
#include "../tools/profiler_include.h"

namespace auto_alg {
namespace detail {
    struct auto_pass_state
    {
        auto_pass_state( const auto_alg::timing_params& params )
            : pwm_iris_controller( 0.4f, 2.0f, 1.0f, 4000 )
        {
            memset( &image_sampling_points.points_float, 0, sizeof( image_sampling_points.points_float ) );
            reset( params );
        }

		auto_alg::detail::pid_controller	pwm_iris_controller;

        int64_t     min_frame_number_distance_ = 2;
        int64_t     max_frame_number_distance_ = 5;
        uint64_t    max_frame_time_distance_ = 100'000;

        int64_t     frame_number_ = 0;
        int64_t     last_frame_number_ = 0;
        uint64_t    last_frame_time_ = 0;


        struct {
            int		current_reference = 128;
            int		calculated_reference = 128;
        } auto_ref;

        struct {
            int		one_push_step_count = 0;
        } wb_temperature;

        auto_alg::impl::auto_focus          focus_onepush_provider;

        auto_alg::impl::auto_sample_points  points_for_wb_temperature;

        auto_alg::impl::image_sampling_data image_sampling_points;


		void    reset( auto_alg::timing_params params )
		{
            frame_number_ = 0;
            last_frame_number_ = 0;
            last_frame_time_ = 0;

            min_frame_number_distance_ = params.min_frame_count_between_runs;
            max_frame_number_distance_ = params.max_frame_count_between_runs;
            max_frame_time_distance_ = params.max_frame_time_between_runs_us;

			auto_ref.calculated_reference = 128;
			auto_ref.current_reference = 128;
			wb_temperature.one_push_step_count = 0;

			pwm_iris_controller.reset();
			focus_onepush_provider.reset();
		}
    };
}
}

namespace
{
    static bool need_brightness_calc( const auto_alg::auto_pass_params& params ) noexcept
    {
        return params.exposure.auto_enabled || params.gain.auto_enabled || params.iris.auto_enabled;
    }
    static bool need_whitebalance_calc( const auto_alg::auto_pass_params& params ) noexcept
    {
        return params.wb.one_push_enabled || params.wb.auto_enabled;
    }
    static bool need_hdr_gain_calc( const img::fourcc img_data_type, const auto_alg::auto_pass_params& params ) noexcept
    {
        return img::is_pwl_fcc( img_data_type ) && params.hdr_gain.enable_auto_hdr_gain_selection;
    }


static float calc_possible_automatic_reduction( const auto_alg::property_cont_exposure& exposure, int exp_val )
{
    return exp_val / (float)exposure.min;
}

static float calc_possible_automatic_reduction( const auto_alg::property_cont_gain& gain, float gain_val )
{
    if( gain.is_gain_db )
    {
        return std::exp( gain_val / gain.gain_db_multiplier * logf( 2.0f ) );
    }
    else
    {
        // Detect CMOS cameras: Expect gain to be multiplier
        return (gain_val / gain.min);
    }
}


static int calc_adjusted_auto_reference_( float factor_of_y_values_greater_240, int user_reference, int calculated_reference )
{
    int reference_value = calculated_reference;
    // decrease reference if to much high tones
    if( factor_of_y_values_greater_240 > 0.15f )
    {
        reference_value -= (int)std::min( 8.f, (40.0f * factor_of_y_values_greater_240 - 4.0f + 0.5f) );
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

static int		calc_auto_reference( const auto_alg::auto_pass_params& params, auto_alg::auto_pass_state& state, float factor_y_vgt240 )
{
    if( factor_y_vgt240 >= 0.0f && params.enable_highlight_reduction && (params.exposure.auto_enabled || params.gain.auto_enabled || params.iris.auto_enabled) )
    {
        state.auto_ref.current_reference = calc_adjusted_auto_reference( factor_y_vgt240, params.exposure_reference.val,
                                                                         state.auto_ref.current_reference, state.auto_ref.calculated_reference );
        return state.auto_ref.current_reference * params.exposure_reference.val / 128;
    }
 
    state.auto_ref.current_reference = params.exposure_reference.val;
    return params.exposure_reference.val;
}

static auto_alg::impl::gain_exposure_iris_values		auto_alg_for_brightness_adjust( auto_alg::auto_pass_state& state, 
                                                                                        const auto_alg::auto_pass_params& exp,
                                                                                        const auto_alg::impl::resulting_brightness& brightness_params )
{
    const int reference = calc_auto_reference( exp, state, brightness_params.factor_y_vgt240 );
    auto_alg::property_cont_iris iris_tmp = exp.iris;
    if( exp.iris.is_pwm_iris )
    {
        iris_tmp.auto_enabled = false;
    }

    auto_alg::impl::gain_exposure_iris_values res = auto_alg::impl::calc_auto_gain_exposure_iris( brightness_params.brightness, reference / 255.f, exp.gain, exp.exposure, iris_tmp );
    if( exp.iris.is_pwm_iris && exp.iris.auto_enabled )
    {
        float corrected_brightness = brightness_params.brightness * 255;

        // If gain and/or exposure are on auto, reduce the brightness according to how much lower they could go
        if( exp.gain.auto_enabled )
        {
            corrected_brightness /= calc_possible_automatic_reduction( exp.gain, res.gain );
        }
        if( exp.exposure.auto_enabled )
        {
            corrected_brightness /= calc_possible_automatic_reduction( exp.exposure, res.exposure );
        }
        res.iris = auto_alg::impl::calc_auto_pwm_iris( corrected_brightness, exp.exposure_reference.val, exp.iris, state.pwm_iris_controller );
    }
    return res;
}


struct  auto_whitebalance_temperature_result
{
    int new_temperature = 0;
    bool keep_onepush_running = false;
};

static auto auto_whitebalance_temperature( auto_alg::auto_pass_state& state,
    const auto_alg::impl::auto_sample_points& points,
    const auto_alg::whitebalance_values& wb
) -> auto_whitebalance_temperature_result
{
	// this construct makes that we can use the already passed in struct for software applied wb devices and only need to make a copy of the array 
    // for hardware applied wb devices
	const auto_alg::impl::auto_sample_points* p_tmp_points = nullptr;
	if( !wb.is_software_whitebalance )
	{
		for( int i = 0; i < points.cnt; ++i )
		{
			auto sample = points.samples[i];

			// we have to scale the points back to the not-white-balanced source to get the right values
			// software applied wb is applied later to the image, so we don't need to revert the image to the 'pre-'-white-balanced source
			sample.rr = (uint8_t)CLIP( sample.rr / wb.channels.r, 0, 0xFF );
			sample.gr = (uint8_t)CLIP( sample.gr / wb.channels.g, 0, 0xFF );
			sample.bb = (uint8_t)CLIP( sample.bb / wb.channels.b, 0, 0xFF );
			sample.gb = (uint8_t)CLIP( sample.gb / wb.channels.g, 0, 0xFF );

			state.points_for_wb_temperature.samples[i] = sample;
		}
		state.points_for_wb_temperature.cnt = points.cnt;

		p_tmp_points = &state.points_for_wb_temperature;
	}
	else
	{
		p_tmp_points = &points;
	}
	// we reference the points array here, because it may be too large for small stacks
	const auto& actually_used_temperature_points = *p_tmp_points;

	bool keep_onepush_running = false;
	int new_temperature = 0;
	if( wb.one_push_enabled )
	{
		new_temperature = auto_alg::impl::calc_temperature_for_pixels( actually_used_temperature_points, wb.temperature.auto_min, wb.temperature.auto_max, wb.temperature.temperature_to_factors_arr );
		if( new_temperature == -1 )
		{
			new_temperature = wb.temperature.current;
		}
		if( --state.wb_temperature.one_push_step_count <= 0 )
		{
			state.wb_temperature.one_push_step_count = 5;
			keep_onepush_running = false;
		}
		else
		{
			keep_onepush_running = true;
		}
	}
	else
	{
		new_temperature = auto_alg::impl::calc_temperature_auto_step( actually_used_temperature_points, wb.temperature.current, wb.temperature.auto_min, wb.temperature.auto_max, wb.temperature.temperature_to_factors_arr );
		keep_onepush_running = false;
	}

    return auto_whitebalance_temperature_result{ new_temperature, keep_onepush_running };
}

static auto_alg::wb_results     exec_auto_whitebalance_steps_on_pixels( auto_alg::auto_pass_state& state,
    const auto_alg::impl::auto_sample_points& points, 
    const auto_alg::whitebalance_values& wb
)
{
	auto_alg::wb_results rval;

    bool param_changed = false;

    if( wb.temperature_mode && wb.temperature.temperature_to_factors_arr != nullptr )
    {
        auto temp_res = auto_whitebalance_temperature( state, points, wb );

        rval.channels = auto_alg::impl::calc_whitebalance_values( temp_res.new_temperature, wb.temperature.temperature_to_factors_arr );
        rval.temperature = temp_res.new_temperature;
		if( wb.one_push_enabled ) {
			rval.one_push_still_running = temp_res.keep_onepush_running;
		}
        param_changed |= temp_res.new_temperature != wb.temperature.current;
    }
    else
    {
        auto call_auto_wb = [&] {
            if( wb.is_software_whitebalance ) {
                return auto_alg::impl::auto_whitebalance_soft( points, wb.channels );
            } else {
                return auto_alg::impl::auto_whitebalance_cam( points, wb.channels );
            }
        };

        auto [done, res_rgb] = call_auto_wb();

        rval.channels = res_rgb;
        if( wb.one_push_enabled ) {
			rval.one_push_still_running = !done;
		}
        param_changed = rval.channels.r != wb.channels.r || rval.channels.g != wb.channels.g || rval.channels.b != wb.channels.b;
    }
    
    rval.wb_changed = param_changed || wb.one_push_enabled != rval.one_push_still_running;
	
    return rval;
}


static auto_alg::wb_results     exec_auto_whitebalance_steps_on_pixels_float(
    const auto_alg::impl::image_sampling_points_rgbf& points,
    const auto_alg::whitebalance_values& wb
)
{
    assert( !(wb.temperature_mode && wb.temperature.temperature_to_factors_arr != nullptr) );   // others currently not implemented
    assert( wb.is_software_whitebalance );   // others currently not implemented

    auto [done, res_rgb] = auto_alg::impl::auto_whitebalance_soft( points, wb.channels );

    const bool rgb_values_changed = res_rgb.r != wb.channels.r || res_rgb.g != wb.channels.g || res_rgb.b != wb.channels.b;

    auto_alg::wb_results rval = { rgb_values_changed, res_rgb, false, /* temperature */ 0 };
    if( wb.one_push_enabled ) {
        rval.one_push_still_running = !done;
        rval.wb_changed = rval.wb_changed || rval.one_push_still_running;
    }
    return rval;
}

static auto_alg::wb_results     exec_auto_whitebalance_steps_on_pixels( auto_alg::auto_pass_state& state,
    const auto_alg::impl::image_sampling_data& points,
    const auto_alg::whitebalance_values& wb
)
{
    DUTIL_PROFILE_FUNCTION();
    if( !points.is_float ) {
        return exec_auto_whitebalance_steps_on_pixels( state, points.points_int, wb );
    } else {
        return exec_auto_whitebalance_steps_on_pixels_float( points.points_float, wb );
    }
}

struct color_img_auto_results
{
    auto_alg::impl::resulting_brightness    brightness_res = auto_alg::impl::resulting_brightness::invalid();
    auto_alg::wb_results                    wb_res{};
    auto_alg::impl::auto_hdr_gain_result  pwl_res{};
};


static color_img_auto_results     exec_color_image_auto( auto_alg::auto_pass_state& state,
    const img::img_descriptor& img_data,
    const auto_alg::auto_pass_params& params
)
{
    // 1. calculate the sampling points

    color_img_auto_results rval;
    if( !auto_alg::impl::auto_sample_by_img( img_data, state.image_sampling_points ) ) {
        return rval;
    }

    // 2. apply color matrix values to it
    apply_software_clrmtx_to_sampling_data( state.image_sampling_points, params.clr );

    if( params.wb.auto_enabled || params.wb.one_push_enabled )
    {
        // calculate whitebalance values
        rval.wb_res = exec_auto_whitebalance_steps_on_pixels( state, state.image_sampling_points, params.wb );
    }
    else
    {
        rval.wb_res.channels = params.wb.channels;  // we copy the passed in whitebalance values here for the is_software_whitebalance step
    }

    if( !(need_brightness_calc( params ) || need_hdr_gain_calc( img_data.fourcc_type(), params )) ) {
        return rval;
    }

    // if the whitebalance values get applied later in the transform stage, we apply the calculated whitebalance values here to the sampling points
    if( params.wb.is_software_whitebalance ) {
        // apply calculated whitebalance on src data to get better brightness mapping
        apply_software_wb_to_sampling_data( state.image_sampling_points, rval.wb_res.channels );
    }

    if( need_hdr_gain_calc( img_data.fourcc_type(), params ) )
    {
        // if we need to calc pwl window data, we do it here
        assert( state.image_sampling_points.is_float );

        rval.pwl_res = auto_alg::impl::auto_hdr_gain( params.hdr_gain, state.image_sampling_points.points_float );
    }

    if( !need_brightness_calc( params ) )
    {
        return rval;
    }

    // this calculates the brightness information for the later auto-exposure/gain steps

    rval.brightness_res = calc_resulting_brightness_params( state.image_sampling_points );
    return rval;
}

static bool is_accepted_mono( img::fourcc fcc ) noexcept
{
    switch( fcc )
    {
    case img::fourcc::MONO8:
    case img::fourcc::MONO16:
    case img::fourcc::MONOFloat:
    case img::fourcc::MONO10:
    case img::fourcc::MONO10_SPACKED:
    case img::fourcc::MONO10_MIPI_PACKED:
    case img::fourcc::MONO12:
    case img::fourcc::MONO12_PACKED:
    case img::fourcc::MONO12_MIPI_PACKED:
    case img::fourcc::MONO12_SPACKED:
        return true;
    default:
        return false;
    }
}

static color_img_auto_results     exec_brightness_and_wb_calc( auto_alg::auto_pass_state& state,
                                                                        const img::img_descriptor& img_data,
                                                                        const auto_alg::auto_pass_params& params )
{
    if( is_accepted_mono( img_data.fourcc_type() ) )
    {
        // Mono-image
        if( need_brightness_calc( params ) ) {
            return color_img_auto_results{ auto_alg::impl::auto_sample_mono_img( img_data ) };
        }
    }
    else if( img::is_pwl_fcc( img_data.fourcc_type() ) )
    {
        if( need_whitebalance_calc( params ) || params.hdr_gain.enable_auto_hdr_gain_selection || need_brightness_calc( params ) ) {
            return exec_color_image_auto( state, img_data, params );
        }
    }
    else if( auto_alg::impl::can_auto_sample_by_img( img_data.fourcc_type() ) )
    {
        if( need_whitebalance_calc( params ) || need_brightness_calc( params ) ) {
            return exec_color_image_auto( state, img_data, params );
        }
    }

    // fall through for unknown format or not needed stuff
    return {};
}

}

/*
 *	Returns true when the auto algorithms will be run on this frame.
 * Note: This increments the local frame number
 */
static bool is_auto_pass_run( auto_alg::auto_pass_state& state, uint64_t time_now, int64_t frame_number ) noexcept
{
    const auto current_frame_number = frame_number;
    if( state.last_frame_time_ == 0 || state.last_frame_time_ > time_now || current_frame_number < state.last_frame_number_ ) {
        // these indicate reset state or some curious time cases
        return true;
    }

    const auto frame_count_diff = current_frame_number - state.last_frame_number_;
    if( frame_count_diff < state.min_frame_number_distance_ ) {
        return false;
    }

    const auto time_since_last_run = time_now - state.last_frame_time_;
    if( time_since_last_run > state.max_frame_time_distance_ ) {
        return true;
    }
    if( frame_count_diff > state.max_frame_number_distance_ ) {
        return true;
    }
    return false;
}

bool auto_alg::should_prepare_auto_pass_step( auto_pass_state& state, const auto_pass_params& params ) noexcept
{
    state.frame_number_ = params.frame_number;
    return is_auto_pass_run( state, params.time_point, params.frame_number ) || state.focus_onepush_provider.is_auto_alg_run_needed( params.focus_onepush_params );
}

auto_alg::auto_pass_results	    auto_alg::auto_pass( auto_pass_state& state, const img::img_descriptor& img_data, const auto_pass_params& params )
{
    auto_pass_results rval = {};

    if( state.focus_onepush_provider.is_auto_alg_run_needed( params.focus_onepush_params ) )
    {
        rval.focus_value = params.focus_onepush_params.device_focus_val;

        state.focus_onepush_provider.auto_alg_run( params.time_point, img_data, params.focus_onepush_params, params.sensor_offset, params.pixel_dim, rval.focus_value );

        rval.focus_onepush_still_running = state.focus_onepush_provider.is_running();
        rval.focus_changed = rval.focus_value != params.focus_onepush_params.device_focus_val;
    }

    if( state.frame_number_ != params.frame_number ) {
        if( !is_auto_pass_run( state, params.time_point, params.frame_number ) ) {
            return rval;
        }
    }

    state.frame_number_ = params.frame_number;
    state.last_frame_time_ = params.time_point;
    state.last_frame_number_ = params.frame_number;

    const bool need_brightness_calculations = need_brightness_calc( params );
    const bool need_whitebalance_calculations = need_whitebalance_calc( params );

    if( !need_whitebalance_calculations && !need_brightness_calculations && !need_hdr_gain_calc( img_data.fourcc_type(), params ) ) {
        return rval;
    }

    DUTIL_PROFILE_SECTION( "auto_alg::auto_pass running auto-stuff" );

    img::img_descriptor img_data_roi = img_data;

    img::rect brightness_roi = img::clip_to_img_desc_region( params.brightness_roi, params.sensor_offset, params.pixel_dim, img_data );
    if( !brightness_roi.is_null() )
    {
        img_data_roi = img::make_safe_img_view( img_data, brightness_roi );
    }

    // This assigns rval.wb if needed and calculates brightness as needed
    const auto results = exec_brightness_and_wb_calc( state, img_data_roi, params );
    rval.wb = results.wb_res;
    if( results.pwl_res.value_changed ) {
        rval.hdr_gain_selection_changed = true;
        rval.hdr_gain_selection_value = results.pwl_res.hdr_gain;
    }

    if( results.brightness_res.brightness < 0.f ) { // we can quit here if brightness is not needed
        return rval;
    }

    rval.image_brightness = results.brightness_res.brightness;

    const auto res = auto_alg_for_brightness_adjust( state, params, results.brightness_res );
    if( params.exposure.val != res.exposure ) {
        rval.exposure_changed = true;
		rval.exposure_value = res.exposure;
    }
	if( params.gain.value != res.gain ) {
		rval.gain_changed = true;
		rval.gain_value = res.gain;
	}
	if( params.iris.val != res.iris ) {
		rval.iris_changed = true;
		rval.iris_value = res.iris;
	}
    return rval;
}

void    auto_alg::reset_auto_pass_context( auto_pass_state& state, const auto_alg::timing_params& params )
{
	state.reset( params );
}

auto_alg::wb_channel_factors const* auto_alg::get_temperature_map_for_sensor( sensor_type sensor )
{
    return auto_alg::detail::get_temperature_map_for_sensor( sensor );
}

auto_alg::wb_channel_factors auto_alg::calc_whitebalance_values_for_temp( int temperature, const wb_channel_factors* arr )
{
    return auto_alg::impl::calc_whitebalance_values( temperature, arr );
}

auto_alg::auto_pass_state* auto_alg::allocate_auto_pass_state( const timing_params& create_params )
{
    return new auto_alg::detail::auto_pass_state( create_params );
}

void auto_alg::deallocate_auto_pass_state( auto_pass_state* context )
{
    delete context;
}

