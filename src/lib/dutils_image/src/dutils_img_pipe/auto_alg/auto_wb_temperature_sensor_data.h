
#pragma once

#include <dutils_img_pipe/auto_alg_params.h>

namespace auto_alg
{
    namespace detail {
        wb_channel_factors const*  get_temperature_map_for_sensor( sensor_type sensor );
    }
}