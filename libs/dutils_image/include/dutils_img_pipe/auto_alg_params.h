
#ifndef AUTO_ALG_PARAMS_INC__
#define AUTO_ALG_PARAMS_INC__

#pragma once

#include <dutils_img/dutils_img.h>
#include <dutils_img/image_transform_data_structs.h>      // img::color_matrix

namespace auto_alg
{
    using img::color_matrix_float;

    struct property_cont_exposure
    {
        bool	auto_enabled = false;

        int		min = 0;
        int		max = 0;

        int		val = 0;

        // Always 1 in legacy cameras
        // In GigE Cameras this is either fetched from GenICam IInteger 'ExposureGranularity' or calculated via 'FPS', 'Height' and 'MinVerticalBlanking'
        // @See genicam_exposure_granularity.cpp in the GigECam driver
        int		granularity = 1;
    };

    struct property_cont_gain
    {
        bool	auto_enabled = false;

        float	min = 0;
        float	max = 0;

        float	value = 0;

        // this is dependent on what the device supports. Most current cameras have a db gain range exported (e.g. [0.0;48.0f])
        // there are devices that export 'Gain (dB/100)', that must be transformed to the right range (e.g. dividing by 100)
        bool    is_gain_db = true;
        float   gain_db_multiplier = 6.0f;  // According to Tim the db mul factor for some sensors is 6.f (and not 3.01f as previously implemented)
    };

    struct property_cont_iris
    {
        bool	auto_enabled = false;

        int		min = 0;
        int		max = 0;

        int		val = 0;

        bool	is_pwm_iris = false;    // There is a GigE-Camera with a PWMIris IInteger GenICam element
        double	camera_fps = 60.;       // Used only when is_pwm_iris == true
    };

    struct color_matrix_params
    {
        color_matrix_float	mtx;
        bool	            enabled;
    };

    struct wb_channel_factors
    {
        float r = 1.f;
        float g = 1.f;
        float b = 1.f;      // range [0.f;4.0f[  
    };

    struct hdr_gain_selection
    {
        bool    enable_auto_hdr_gain_selection = false;
        
        img::pwl_transform_params    transform_param;
        float   hdr_gain_auto_reference = 0.5f;     // [0;1.f] default 0.5f
    };

    struct auto_focus_params
    {
        bool    enable_focus = false;

        int     device_focus_val = 0;           // the currently set focus value, this should be the actual value, that is present in the device

        bool    is_end_cmd = false;                 // when set, the auto_focus code stops the current auto run
        bool    is_run_cmd = false;                 // when set, the auto_focus code resets and takes the run_cmd_params to init itself
        struct run_cmd_param_struct
        {
            img::rect   roi = {};                   // user roi, default == { 0, 0, 0, 0 }, this means full image
            int         focus_range_min = 0;        // minimum focus range as provided by the device/user
            int         focus_range_max = 0;        // maximum  ^^
            int         focus_device_speed = 500;   // Legacy and GigECam = 500 (will most likely never change ...)
            int         auto_step_divisor = 4;      // Legacy cam = 4, GigECam this is read from  IInteger 'FocusAutoStepDivisor' otherwise 4
            bool        suggest_sweep = false;      // Legacy cam = false, GigECam this is read from IInteger 'FocusAutoSweepHint' otherwise false
        } run_cmd_params;
    };

    enum class sensor_type
    {
        Unknown,
        MT9P031,
        MT9V024,
        MT9M021,
        ICX445AQA,
        ICX274AQ,
        ICX618AQA,

        MT9P031_Z30,
        MT9P006,

        IMX236,
    };

    constexpr const int TemperatureMap_ElementCount = 76;
}

#endif