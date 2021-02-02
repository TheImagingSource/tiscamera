
#pragma once

#include <cstdint>

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4201 ) // nonstandard extension used : nameless struct/union
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#endif

namespace img
{
    struct single_channel_lut
    {
        uint8_t	    table_y8[256];
        uint16_t	table_y16[256 * 256];
    };

    struct color_matrix
    {
        union
        {
            struct
            {
                int16_t r_rfac, r_gfac, r_bfac,
                    g_rfac, g_gfac, g_bfac,
                    b_rfac, b_gfac, b_bfac;
            };
            int16_t fac[9];
        };

        static constexpr color_matrix get_defaults() noexcept { return get_increased_saturation(); }
        static constexpr color_matrix get_increased_saturation() noexcept {
            return color_matrix{  90, -13, -13,
                                 -13,  90, -13, 
                                 -13, -13,  90 };
        }
        static constexpr color_matrix get_neutral() noexcept {
            return color_matrix{ 64,  0,  0,
                                  0, 64,  0,
                                  0,  0, 64 };
        }

        constexpr int16_t& operator[]( int idx ) noexcept { return fac[idx]; }
        constexpr int16_t operator[]( int idx ) const noexcept { return fac[idx]; }
    };

    struct tonemapping_params
    {
        bool    enable = false;                     // enable/disable this property
        bool    disable_auto = false;               // auto generation of mul/add/pow/lum_avg factors
        float   intensity = 1.f;                    // [-8.f;+8.f]
        float	global_brightness_factor = 0.f;	    // [0.f;1.f]	(default 0)

        float   mul_factor = 0.f;
        float   add_factor = 0.f;
        float   pow_factor = 0.f;
        float	lum_avg = 0.f;

        static constexpr tonemapping_params get_defaults() noexcept { return tonemapping_params{}; }
    };

    struct pwl_transform_params
    {
        float   window_in_bits = 24.f;       // [1.f;24.f]  step=0.1f
        float   level_in_bits = 0.f;        // [-24.f;24.f]  step=0.1f
    };
}

#ifdef _MSC_VER
#pragma warning( pop )
#else
#pragma GCC diagnostic pop
#endif