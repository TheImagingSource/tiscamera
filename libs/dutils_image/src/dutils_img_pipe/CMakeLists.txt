

add_library(dutils_img_pipe_auto STATIC

    "auto_alg/auto_alg_pass_itf.cpp"
    "auto_alg/auto_alg_pass_itf.h"

    "auto_alg/auto_alg.cpp"
    "auto_alg/auto_alg.h"
    "auto_alg/auto_sample_image.cpp"
    "auto_alg/auto_sample_image.h"
    "auto_alg/auto_exposure.cpp"
    "auto_alg/auto_exposure.h"
    "auto_alg/auto_focus.cpp"
    "auto_alg/auto_focus.h"
    "auto_alg/auto_wb_software_applied_int.cpp"
    "auto_alg/auto_wb_cam.cpp"

    "auto_alg/auto_wb_temperature.cpp"
    "auto_alg/auto_wb_temperature.h"
    "auto_alg/auto_wb_temperature_sensor_data.cpp"
    "auto_alg/auto_wb_temperature_sensor_data.h"
    "auto_alg/auto_wb_temperature_gen.cpp"
    "auto_alg/auto_wb_temperature_gen.h"

    "auto_alg/empia_pattern_fix.cpp"
    "auto_alg/empia_pattern_fix.h"
    "auto_alg/pid_controller.cpp"
    "auto_alg/pid_controller.h"

    "auto_alg/image_sampling_float.h"
    "auto_alg/image_sampling_float.cpp"
    "auto_alg/auto_wb_software_applied_float.cpp"

    "auto_alg/image_sampling_u8.h"
    "auto_alg/image_sampling_u8.cpp"
    "auto_alg/auto_hdr_gain.h"
    "auto_alg/auto_hdr_gain.cpp"
)

target_link_libraries( dutils_img_pipe_auto 
PRIVATE
    dutils_img::project_warnings
    dutils_img::project_options

    dutils_img::base
    dutils_img::dutils_img_filter_c
PUBLIC
    dutils_img::img
)

add_library( dutils_img::pipe_auto ALIAS dutils_img_pipe_auto )