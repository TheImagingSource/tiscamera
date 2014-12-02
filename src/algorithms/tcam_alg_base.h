
#ifndef TCAM_ALG_BASE_H
#define TCAM_ALG_BASE_H

#include "base_types.h"

namespace tcam
{

struct tcam_whitebalance_values
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

        // const auto_alg::auto_wb* temperature_to_factors_arr;
    } temperature;
};


struct tcam_auto_alg_exposure
{
    tcam_value_int value;
    bool do_auto;
};


struct tcam_auto_alg_gain
{
    tcam_value_int value;
    bool   do_auto;
    double steps_to_double_brightness;
    bool   is_db_gain;
};


struct tcam_auto_alg_iris
{
    tcam_value_int value;
    bool   do_auto;
    double camera_fps;
    bool is_pwm_iris;
};


#ifndef RECT

struct RECT
{
    long left;
    long top;
    long right;
    long bottom;
};

#endif

struct tcam_auto_focus_params
{
    int device_focus_val;           // the currently set focus value, this should be the actual value, that is present in the device

    bool is_end_cmd;                 // when set, the auto_focus code stops the current auto run
    bool is_run_cmd;                 // when set, the auto_focus code resets and takes the run_cmd_params to init itself
    struct run_cmd_param_struct {
        RECT    roi;                    // user roi, must { 0, 0, 0, 0 } to be ignored
        int     focus_range_min;        // minimum focus range as provided by the device/user
        int     focus_range_max;        // maximum  ^^
        int     focus_device_speed;     // device speed, currently set to 500
        int     auto_step_divisor;      // supplied by the device, otherwise currently 4
        bool    suggest_sweep;          // should be default false, otherwise suggested by the device
    } run_cmd_params;
};


enum {
    AUTO_APPLY_DISTANCE = 3,
};

struct tcam_auto_alg_state;
typedef tcam_auto_alg_state* tcam_auto_alg_context;



struct tcam_auto_alg_params
{
    tcam_auto_alg_exposure   exposure;
    tcam_auto_alg_exposure   exposure_reference;
    tcam_auto_alg_gain       gain;
    tcam_auto_alg_iris       iris;
    tcam_whitebalance_values wb;

    bool enable_auto_ref;

    tcam_auto_focus_params focus_onepush_params;

    bool enable_highlight_reduction;

};


struct tcam_auto_alg_results
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

struct tcam_create_auto_alg_params
{
    int  auto_apply_distance;
    bool add_software_onepush_focus;
    bool is_software_applied_wb;
};

} /* namespace tcam */

#endif /* TCAM_ALG_BASE_H */
