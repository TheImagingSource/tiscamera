
#pragma once

#include <algorithms/auto_alg_params.h>

namespace auto_alg
{
    namespace detail {
        wb_channel_factors const*  get_temperature_map_for_sensor( sensor_type sensor );
    }
}