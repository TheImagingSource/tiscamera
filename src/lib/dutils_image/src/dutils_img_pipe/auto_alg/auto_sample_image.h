
#pragma  once

#include <dutils_img_pipe/auto_alg_params.h>

#include <dutils_img/pixel_structs.h>

namespace auto_alg::impl
{
    using pixel = img::pixel_type::B8G8R8;
    using RGBf = img::pixel_type::RGBf;

    struct auto_sample_entry
    {
        uint8_t	rr, gr, bb, gb;

		constexpr auto to_pixel() const noexcept {
			return pixel{ bb, uint8_t( (gr + gb) / 2 ), rr };
		}
        constexpr void operator=( pixel pix ) noexcept {
            rr = pix.r;
            gr = pix.g;
            bb = pix.b;
            gb = pix.g;
        }
    };

    struct auto_sample_points
    {
        int	cnt;
        auto_sample_entry samples[1500];
    };
    
    struct image_sampling_points_rgbf
    {
        int	cnt;
        img::pixel_type::RGBf   samples[1500];
    };

    struct image_sampling_data
    {
        bool    is_float = false;
        union {
            auto_sample_points          points_int;
            image_sampling_points_rgbf  points_float;
        };
    };

    struct resulting_brightness
    {
        float   brightness = 0.5f;
        float   factor_y_vgt240 = 0.f;

        static constexpr resulting_brightness invalid() noexcept { return resulting_brightness{ -1.f, -1.f }; }
    };

    // Sampling functions
    bool                    auto_sample_by_img( const img::img_descriptor& image, image_sampling_data& points );
    resulting_brightness    auto_sample_mono_img( const img::img_descriptor& image );

    img::dim                calc_image_sample_step_dim( const img::img_descriptor& image ) noexcept;
    bool                    can_auto_sample_by_img( img::fourcc fcc ) noexcept;
}


