
#pragma once

#include "../by8/by8_base.h"
#include "auto_alg_params.h"

namespace img {
	struct img_descriptor;
    class auto_focus;
};

namespace auto_alg
{
	struct auto_wb;

	struct whitebalance_values 
	{
		int		r;
		int		g;
		int		b;

		bool	auto_enabled;
		bool	one_push_enabled;

		bool	is_software_applied_wb;

		bool	temperature_mode;

		struct {
			int		current;
			int		auto_min;
			int		auto_max;

			const auto_alg::auto_wb* temperature_to_factors_arr;
		} temperature;
	};

    struct auto_focus_params
    {
        int     device_focus_val;           // the currently set focus value, this should be the actual value, that is present in the device

        bool    is_end_cmd;                 // when set, the auto_focus code stops the current auto run
        bool    is_run_cmd;                 // when set, the auto_focus code resets and takes the run_cmd_params to init itself
        struct run_cmd_param_struct {
            RECT    roi;                    // user roi, must { 0, 0, 0, 0 } to be ignored
            int     focus_range_min;        // minimum focus range as provided by the device/user
            int     focus_range_max;        // maximum  ^^
            int     focus_device_speed;     // device speed, currently set to 500
            int     auto_step_divisor;      // supplied by the device, otherwise currently 4
            bool    suggest_sweep;          // should be default false, otherwise suggested by the device
        } run_cmd_params;
    };

	struct auto_pass_params
	{
        POINT                   sensor_offset;  // the position of the img in sensor coordinates
        SIZE                    pixel_dim;      // the pixel dimensions e.g. binning 4x2    => { 4, 2 }

        RECT                    brightness_roi; // this specifies the brightness_roi in sensor coordinates
                                                // the region is internally clipped to the actual img_desc passed

        property_cont_gain		gain;
		property_cont_exposure	exposure;
        struct {
            int     val;
        } exposure_reference;               // must be in the range of [0;255], default is 128
		property_cont_iris		iris;

		whitebalance_values		wb;

		color_matrix_params		clr;

        auto_focus_params       focus_onepush_params;

		bool					enable_highlight_reduction;
	};

    enum {
        AUTO_APPLY_DISTANCE = 3,
    };

    struct auto_pass_state;
    typedef auto_pass_state* auto_pass_context;

	struct auto_pass_results
	{
		int		exposure;
		int		gain;
		int		iris;

		int		wb_r;
		int		wb_g;
		int		wb_b;
		bool	wb_one_push_still_running;
		int		wb_temperature;

		int		brightness;

        bool    auto_pass_has_run;

        int     focus_value;
        bool    focus_onepush_running;
	};

    struct create_params
    {
        int  auto_apply_distance;
        bool add_software_onepush_focus;
        bool is_software_applied_wb;
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
	auto_pass_results	auto_pass( const img::img_descriptor& data, const auto_pass_params& params, auto_pass_context state );
    
    // should be called each time you re-/start the stream
    void                reinit_auto_pass_context( auto_pass_context context );

    // create a auto_pass context
    auto_pass_context   create_auto_pass_context( void* context_space, size_t context_space_size, const create_params& create_params );
    // destroy the created context
    void                destroy_auto_pass_context( auto_pass_context context );

    // the needed size for the context
    size_t              get_auto_pass_context_size( const create_params& create_params );
};
