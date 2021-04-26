
#pragma once

#include "auto_alg.h"

#include <vector>

namespace auto_alg::impl
{
    // rgb_values_at_6500 must be the rgb values for the temperature at 6500 Kelvin
    std::vector<auto_alg::wb_channel_factors>  create_temperature_table( auto_alg::wb_channel_factors rgb_values_at_6500 );
}