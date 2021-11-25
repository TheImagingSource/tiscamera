
#pragma once

#include <dutils_img/dutils_img.h>

#include "auto_alg_params.h"
#include "dll_export.h"
#include "dutils_img_state_helper.h"

namespace auto_alg
{
    namespace detail {
        struct auto_pass_state;
    }

    using auto_pass_state = detail::auto_pass_state;

    struct whitebalance_values 
    {
        bool    is_software_whitebalance = false;   // if a step after this applies the actual whitebalance and not the camera

        wb_channel_factors channels{ 1.f, 1.f, 1.f };

        bool	auto_enabled = false;
        bool	one_push_enabled = false;

        bool	temperature_mode = false;

        struct {
            int		current = 6500;
            int		auto_min = 3000;
            int		auto_max = 10000;

            const auto_alg::wb_channel_factors* temperature_to_factors_arr = nullptr;
        } temperature;
    };

    struct auto_pass_params
    {
        int64_t                 frame_number = 0;
        uint64_t                time_point = 0;     // in us, note that the start time is unspecified

        img::point              sensor_offset = { 0, 0 };  // the position of the img in sensor coordinates
        img::dim                pixel_dim = { 1, 1 };      // the pixel dimensions e.g. binning 4x2    => { 4, 2 }

        img::rect               brightness_roi = { 0, 0, 0, 0 };    // this specifies the brightness_roi in sensor coordinates
                                                                    // the region is internally clipped to the actual img_desc passed

        property_cont_gain		gain{};           // see struct
        property_cont_exposure	exposure{};       // should be a reasonable exposure value. e.g. units in us
        struct {
            int     val = 128;
        } exposure_reference;               // must be in the range of [0;255], default is 128
        property_cont_iris		iris{};

        whitebalance_values		wb{};

        color_matrix_params		clr{ color_matrix_float::get_defaults(), false };

        auto_focus_params       focus_onepush_params{};

        bool					enable_highlight_reduction = false; // only used, when exposure/gain/iris is enabled

        hdr_gain_selection      hdr_gain{};   // only used when pwl is the input format
    };

    struct wb_results
    {
        bool    wb_changed = false;
        wb_channel_factors  channels = {};

        bool	one_push_still_running = false;
        int		temperature = 0;
    };

    struct auto_pass_results
    {
        bool    exposure_changed = false;
        int		exposure_value = 0;
        bool    gain_changed = false;
		float	gain_value = 0;
		bool    iris_changed = false;
        int		iris_value = 0;

        wb_results wb;

        bool    focus_changed = false;
        int     focus_value = 0;
        bool    focus_onepush_still_running = false;

        bool    hdr_gain_selection_changed = false;
        float   hdr_gain_selection_value = 0.f;

        float   image_brightness = 0;
    };

    struct timing_params
    {
        uint32_t     min_frame_count_between_runs = 2;          // The count of frames to at least wait between runs
        uint32_t     max_frame_count_between_runs = 5;          // The count of frames to at max wait between runs
        uint32_t     max_frame_time_between_runs_us = 100'000;  // The max time between runs (after min_frame_count)
    };

    /** This is the central function which executes the auto algorithm functions on the input img_descriptor
     * The different algorithms take the specific parameters from the according auto_pass structure elements.
     *
     *  The auto_pass_params argument encapsulates all parameters which should be passed in from the property providers.
     * This includes ranges and the current state of the device.
     *
     *  The auto_pass_state argument contains the data the algorithms itself want to keep across calls. Note that the state parameter should be
     * initialized by init_auto_pass_state before any call to auto_pass. 
     *
     *  This function returns the auto_pass_results struct. You should test if the input parameter is not equal to the result entry to determine
     * whether the specific property changed.
     * 
     */
    auto_pass_results	auto_pass( auto_pass_state& state, const img::img_descriptor& data, const auto_pass_params& params );
    
    bool                should_prepare_auto_pass_step( auto_pass_state& state, const auto_pass_params& params ) noexcept;

    auto_pass_state*    allocate_auto_pass_state( const timing_params& create_params = {} );
    void                deallocate_auto_pass_state( auto_pass_state* context );

    // should be called each time you re-/start the stream
    void                reset_auto_pass_context( auto_pass_state& context, const timing_params& create_param = {} );

    wb_channel_factors const* get_temperature_map_for_sensor( sensor_type sensor );
    wb_channel_factors	        calc_whitebalance_values_for_temp( int temperature, const wb_channel_factors* arr );

    using state_ptr = dutils::state_ptr_type<auto_alg::auto_pass_state, auto_alg::deallocate_auto_pass_state>;

    inline state_ptr  make_state_ptr( timing_params params = {} ) {
        return state_ptr{ auto_alg::allocate_auto_pass_state( params ) };
    }
}
