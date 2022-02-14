
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

    // range of the components seems to be [-1;+3]
    struct color_matrix_float
    {
        union
        {
            struct
            {
                float r_rfac, r_gfac, r_bfac,
                    g_rfac, g_gfac, g_bfac,
                    b_rfac, b_gfac, b_bfac;
            };
            float fac[9];
            float fac_3x3[3][3];
        };
        static constexpr color_matrix_float get_defaults() noexcept { return get_increased_saturation(); }
        static constexpr color_matrix_float get_increased_saturation() noexcept {
            return color_matrix_float{ 1.40625f,  -0.203125f, -0.203125f,
                                 -0.203125f,  1.40625f , -0.203125f,
                                 -0.203125f, -0.203125f,  1.40625f };
        }
        static constexpr color_matrix_float get_neutral() noexcept {
            return color_matrix_float{ 1.f, 0.f, 0.f,
                                  0.f, 1.f, 0.f,
                                  0.f, 0.f, 1.f };
        }
        constexpr float& operator[]( int idx ) noexcept { return fac[idx]; }
        constexpr float operator[]( int idx ) const noexcept { return fac[idx]; }
        constexpr float at( int x, int y ) const noexcept { return fac_3x3[x][y]; }
    };

    // this should be faced out in the future
    struct color_matrix_int
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
            int16_t fac_3x3[3][3];
        };

        static constexpr color_matrix_int get_defaults() noexcept { return get_increased_saturation(); }
        static constexpr color_matrix_int get_increased_saturation() noexcept {
            return color_matrix_int{  90, -13, -13,
                                 -13,  90, -13, 
                                 -13, -13,  90 };
        }
        static constexpr color_matrix_int get_neutral() noexcept {
            return color_matrix_int{ 64,  0,  0,
                                  0, 64,  0,
                                  0,  0, 64 };
        }

        constexpr int16_t& operator[]( int idx ) noexcept { return fac[idx]; }
        constexpr int16_t operator[]( int idx ) const noexcept { return fac[idx]; }

        color_matrix_float  to_float() const noexcept {
            color_matrix_float rval;
            for( int i = 0; i < 9; ++i ) {
                rval.fac[i] = fac[i] / 64.f;
            }
            return rval;
        }
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
        float   hdr_gain = 0.f;       // [0.f;120.f]
    };

    struct whitebalance_params
    {
        bool    apply = false;      // entries ignored on false
        float   wb_rr = 1.f;        // [0;4( 1.0 ^= neutral
        float   wb_gr = 1.f;
        float   wb_bb = 1.f;
        float   wb_gb = 1.f;
};
}

#ifdef _MSC_VER
#pragma warning( pop )
#else
#pragma GCC diagnostic pop
#endif