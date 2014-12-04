
#include "tcam_auto_alg.h"

#include <img/img_overlay.h>
#include "auto_alg/auto_alg_pass.h"
#include "dutils_header.h"

#include "tcam.h"

using namespace tcam;

static bool convert_params (const tcam_auto_alg_params& t, auto_alg::auto_pass_params& p)
{
    p.exposure.min = t.exposure.value.min;
    p.exposure.max = t.exposure.value.max;
    p.exposure.val = t.exposure.value.value;
    p.exposure.do_auto = t.exposure.do_auto;

    p.gain.min = t.gain.value.min;
    p.gain.max = t.gain.value.max;
    p.gain.val = t.gain.value.value;
    p.gain.do_auto = t.gain.do_auto;
    p.gain.steps_to_double_brightness = t.gain.steps_to_double_brightness;

    p.iris.min = t.iris.value.min;
    p.iris.max = t.iris.value.max;
    p.iris.val = t.iris.value.value;
    p.iris.do_auto = t.iris.do_auto;
    p.iris.camera_fps = t.iris.camera_fps;
    p.iris.is_pwm_iris = t.iris.is_pwm_iris;

    p.focus_onepush_params.device_focus_val = t.focus_onepush_params.device_focus_val;
    p.focus_onepush_params.run_cmd_params.focus_range_min = t.focus_onepush_params.run_cmd_params.focus_range_min;
    p.focus_onepush_params.run_cmd_params.focus_range_max = t.focus_onepush_params.run_cmd_params.focus_range_max;
    p.focus_onepush_params.run_cmd_params.focus_device_speed = t.focus_onepush_params.run_cmd_params.focus_device_speed;
    p.focus_onepush_params.run_cmd_params.auto_step_divisor = t.focus_onepush_params.run_cmd_params.auto_step_divisor;
    p.focus_onepush_params.run_cmd_params.suggest_sweep = t.focus_onepush_params.run_cmd_params.suggest_sweep;

    p.focus_onepush_params.is_run_cmd = t.focus_onepush_params.is_run_cmd;
    p.focus_onepush_params.is_end_cmd = t.focus_onepush_params.is_end_cmd;


    p.wb.r = t.wb.r;
    p.wb.g = t.wb.g;
    p.wb.b = t.wb.b;

    p.wb.auto_enabled = t.wb.auto_enabled;
    p.wb.one_push_enabled = t.wb.one_push_enabled;
    p.wb.is_software_applied_wb = t.wb.is_software_applied_wb;
    p.wb.temperature_mode = t.wb.temperature_mode;
    p.wb.temperature.current = t.wb.temperature.current;
    p.wb.temperature.auto_min = t.wb.temperature.auto_min;
    p.wb.temperature.auto_max = t.wb.temperature.auto_max;

    p.exposure_reference.val = 128;

    return true;
}


static bool convert_results (const auto_alg::auto_pass_results& r, tcam_auto_alg_results& t)
{

    t.exposure = r.exposure;
    t.gain = r.gain;
    t.iris = r.iris;
    t.wb_r = r.wb_r;
    t.wb_g = r.wb_g;
    t.wb_b = r.wb_b;
    t.wb_one_push_still_running = r.wb_one_push_still_running;
    t.wb_temperature = r.wb_temperature;
    t.brightness = r.brightness;

    t.auto_pass_has_run = r.auto_pass_has_run;
    t.focus_value = r.focus_value;
    t.focus_onepush_running = r.focus_onepush_running;

    return true;
}


void tcam::reinit_auto_pass_context (tcam_auto_alg_context context)
{
    auto_alg::reinit_auto_pass_context(reinterpret_cast<auto_alg::auto_pass_context>(context));
}


static auto_alg::create_params convert_create_params (struct tcam_create_auto_alg_params* params)
{
    auto_alg::create_params p = {};

    if (params != nullptr)
    {
        p.auto_apply_distance = params->auto_apply_distance;
        p.add_software_onepush_focus = params->add_software_onepush_focus;
        p.is_software_applied_wb = params->is_software_applied_wb;
    }
    return p;
}


size_t tcam::get_auto_pass_context_size ()
{
    auto_alg::create_params p = {};
    auto size = auto_alg::get_auto_pass_context_size (p);
    return size;
}


tcam_auto_alg_context tcam::create_auto_pass_context (void* context_space,
                                                      struct tcam_create_auto_alg_params* params)
{
    auto_alg::create_params p = convert_create_params(params);

    auto size = auto_alg::get_auto_pass_context_size (p);

    auto cont = auto_alg::create_auto_pass_context(context_space, size, p);

    return reinterpret_cast<tcam_auto_alg_context>(cont);
}



tcam_auto_alg_results tcam::auto_pass (const struct tcam_image_buffer* buffer,
                                       struct tcam_auto_alg_params* params,
                                       tcam_auto_alg_context state)
{
    auto img = to_img_desc(*buffer);

    auto_alg::auto_pass_params p = {};
    // static auto_alg::auto_pass_state state;

    // auto_alg::auto_pass_context state;

    convert_params(*params, p);

    auto r = auto_alg::auto_pass(img, p, reinterpret_cast<auto_alg::auto_pass_context>(state));

    tcam_auto_alg_results results = {};

    convert_results(r, results);

    return results;
}


void tcam::destroy_auto_pass_context (tcam_auto_alg_context context)
{
    auto_alg::destroy_auto_pass_context(reinterpret_cast<auto_alg::auto_pass_context>(context));
}


void tcam::apply_wb_to_bayer_img (struct tcam_image_buffer* buffer,
                                  unsigned char r,
                                  unsigned char g,
                                  unsigned char b,
                                  unsigned int i)
{
    auto img = to_img_desc(*buffer);

    by8_transform::apply_wb_to_bayer_img(img, r, g, b, g, 0);
}


bool tcam::overlay (const struct tcam_image_buffer* buf,
                    const char* text,
                    unsigned int pos_x,
                    unsigned int pos_y)
{

    img::img_descriptor img = to_img_desc(*buf);
    img::overlay::COLOR fg = img::overlay::WHITE;
    img::overlay::COLOR bg = img::overlay::BLUE;

    POINT pos = {(int)pos_x, (int)pos_y};
    unsigned int scaling = 10;


    img::overlay::RenderText(img, pos, scaling, text, bg, fg, 1);

    return true;
}
