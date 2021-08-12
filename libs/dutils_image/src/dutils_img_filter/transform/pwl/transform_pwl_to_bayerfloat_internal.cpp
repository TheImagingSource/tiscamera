
#include "transform_pwl_to_bayerfloat_internal.h"

#include <memory>
#include <iterator>

namespace
{
    struct PWLControlPoint
    {
        // the values are expected to take the values of the registers in the Sony IMX390
        uint32_t x, y, gain;
    };

    // fixed PWL compression settings used in our MIPI drivers
    static const constexpr PWLControlPoint transform_pwl_control_points[] = {
        {0x000000, 0x00000, 0x1000000},
        {0x000180, 0x00180, 0x0555555},
        {0x000640, 0x00315, 0x01C71C7},
        {0x000E74, 0x003FE, 0x0033975},
        {0x005DC0, 0x004FE, 0x0019CBA},
        {0x011D28, 0x00632, 0x000CE5D},
        {0x033D58, 0x007E3, 0x0001419},
        {0x16E360, 0x0096F, 0x0000A0C},
        {0x4C4B40, 0x00B87, 0x0000506},
        {0xFFFFFF, 0x00F0E, 0x0000000}
    };

    static const constexpr int pedestal_level = 240;
    static const constexpr uint32_t gain1 = 0x1000000; // constant for a gain factor of 1.0
    static const constexpr int input_value_range = 1 << 12;
    static const constexpr int output_value_range = 1 << 24;

    std::unique_ptr<float[]> create_lut_for_transform_pwl_to_float()
    {
        std::unique_ptr<float[]> lut( new float[input_value_range] );
        for( int i = 0; i < input_value_range; ++i )
        {
            lut[i] = transform_pwl_internal::transform_pwl_to_float_single_value( i );
        }
        return lut;
    }
}

uint32_t transform_pwl_internal::transform_pwl_to_int_single_value( int value )
{
    int y = value - pedestal_level;
    if( y < 0 ) y = 0;

    // find last control point whose y coordinate is less than or equal to given y
    auto controlPointIterator = std::cbegin( transform_pwl_control_points );
    for( ; controlPointIterator != std::cend( transform_pwl_control_points ); ++controlPointIterator )
    {
        if( (int)controlPointIterator->y > y )
        {
            break;
        }
    }
    // advance back, since we are now at the successor of the element looked for
    --controlPointIterator;
    if( controlPointIterator->gain )
    {
        return uint32_t( (int64_t)(y - controlPointIterator->y) * gain1 / controlPointIterator->gain + controlPointIterator->x );
    }
    else
    {
        return output_value_range - 1;
    }
}

float transform_pwl_internal::transform_pwl_to_float_single_value( int value )
{
    return transform_pwl_to_int_single_value( value ) / float( output_value_range ); // normalize to float range [0 ... 1)
}

const float*      transform_pwl_internal::get_lut_for_transform_pwl_to_float()
{
    static auto lut = create_lut_for_transform_pwl_to_float();
    return lut.get();
}
