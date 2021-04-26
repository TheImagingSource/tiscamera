
#include "auto_wb_temperature_gen.h"

#include <algorithm>
#include <cmath>

namespace
{

static auto_alg::wb_channel_factors    normalize( auto_alg::wb_channel_factors wb_values )
{
    float tmp = std::min( wb_values.r, std::min( wb_values.g, wb_values.b ) );
    if( tmp == 0.0f ) {
        tmp = 1.0f;
    }
    return auto_alg::wb_channel_factors{ wb_values.r / tmp, wb_values.g / tmp, wb_values.b / tmp };
}


// approximation to sRGB -> seems to best fit individual camera color space overall
static auto_alg::wb_channel_factors    calc_rgb_factors_from_temperature( double tmpKelvin )
{
    // Temperature must fall between 1000 and 40000 degrees
    if( tmpKelvin < 1000 ) tmpKelvin = 1000;
    if( tmpKelvin > 40000 ) tmpKelvin = 40000;

    // All calculations require tmpKelvin \ 100, so only do the conversion once
    tmpKelvin = tmpKelvin / 100.0;

    // Calculate each color in turn

    double r = 0;
    double g = 0;
    double b = 0;

    // First: red
    if( tmpKelvin <= 66 ) {
        r = 255;
    }
    else
    {
        // Note: the R-squared value for this approximation is .988
        double tmpCalc = (329.698727446 * pow( tmpKelvin - 60, -0.1332047592 ));
        r = tmpCalc;
        if( r < 1 ) r = 1;
        if( r > 255 ) r = 255;
    }

    // Second: green
    if( tmpKelvin <= 66 )
    {
        // Note: the R-squared value for this approximation is .996
        double tmpCalc = (99.4708025861 * log( tmpKelvin ) - 161.1195681661);
        g = tmpCalc;
        if( g < 1 ) g = 1;
        if( g > 255 ) g = 255;
    }
    else
    {
        // Note: the R-squared value for this approximation is .987
        double tmpCalc = (288.1221695283 * pow( tmpKelvin - 60, -0.0755148492 ));
        g = tmpCalc;
        if( g < 1 ) g = 1;
        if( g > 255 ) g = 255;
    }

    // Third: blue
    if( tmpKelvin >= 66 )
    {
        b = 255;
    }
    else if( tmpKelvin <= 19 )
    {
        b = 1;
    }
    else
    {
        //Note: the R-squared value for this approximation is .998
        double tmpCalc = 138.5177312231 * log( tmpKelvin - 10 ) - 305.0447927307;

        b = tmpCalc;
        if( b < 1 ) b = 1;
        if( b > 255 ) b = 255;
    }

    return {
        (float)(255.0 / r),
        (float)(255.0 / g),
        (float)(255.0 / b)
    };
}
}

std::vector<auto_alg::wb_channel_factors> auto_alg::impl::create_temperature_table( auto_alg::wb_channel_factors rgb_values_at_6500 )
{
    auto in_values = normalize( rgb_values_at_6500 );

    std::vector<auto_alg::wb_channel_factors> wbData( 76 );

    int index = 0;
    for( float temperature = 2500; temperature <= 10000; temperature += 100 )
    {
        auto vals = calc_rgb_factors_from_temperature( temperature );

        float fr = vals.r * in_values.r;
        float fg = vals.g * in_values.g;
        float fb = vals.b * in_values.b;

        wbData[index] = normalize( { fr, fg, fb } );

        ++index;
    }
    return wbData;
}
