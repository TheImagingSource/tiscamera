
#pragma once

#include "../image_transform_base.h"
#include "../by8/by8_base.h"

namespace img
{
	struct img_transform_params {
		unsigned int	cpu_features;

		struct {
			by8_transform::color_matrix	mtx;
			bool						use_mtx;
		} by8_params;

        struct  
        {
            bool    apply_wb;
            int     r;
            int     gr;
            int     b;
            int     gb;
        } wb;

		int		saturation;		// [-64,192], saturation factor is (x + 64.0) / 64.0, so 0 is factor 1.0f
		int		hue;
        int		sharpness;

		int		denoise_amount;

		bool	enable_green_pattern_fix;
		bool	flip_h;
        bool    enable_by8_matrix_code;

        int		brightness;
		int		contrast;
        int     gamma;
		struct {
			bool        use_lut;
			byte	    table_y8[256];
            uint16_t	table_y16[256 * 256];
		} lut;

		void*	scratch_space;
        int     scratch_space_len;
	};

	void	transform_pipe( img::img_descriptor& dest, img::img_descriptor& src, const img_transform_params& params );
	int	    get_needed_scratchspace_size( const img_type& dest_img, const img_type& src_img );
};
