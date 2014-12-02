
#ifndef BY8_APPLY_WHITEBALANCE_H_INC_
#define BY8_APPLY_WHITEBALANCE_H_INC_

#pragma once

#include "../image_transform_base.h"

namespace by8_transform
{
	/* Applies the passed white-balance values to either the image itself or the destination image.
	 * white balance values must be in the range [0,0xFF] with 64 being the neutral value (* 1).
	 * The functions do not check if all white balance values are 1, in which case either not touching
	 * the values or just a memcpy could be sufficient.
	 * The functions do not check for availability of SSE2.
	 * Currently SSE2 is implemented with unaligned reads/stores and may be slow on CPUs which
	 * run faster with aligned stores/reads. (Core Duo/Core Quad)
	 *
	 * Parameters for the (dest, src) version:
	 		src.length, src.dim_x and src.dim_y are ignored.
			The pattern of the image to transform is extracted from src.type.
	 */

	void		apply_wb_to_bayer_img_c( img::img_descriptor& data, byte wb_r, byte wb_g, byte wb_b );
	void		apply_wb_to_bayer_img_c( img::img_descriptor& data, byte wb_r, byte wb_gr, byte wb_b, byte wb_gb );

	void		apply_wb_to_bayer_img_c( img::img_descriptor& dest, const img::img_descriptor& src, byte wb_r, byte wb_g, byte wb_b );
	void		apply_wb_to_bayer_img_c( img::img_descriptor& dest, const img::img_descriptor& src, byte wb_r, byte wb_gr, byte wb_b, byte wb_gb );

    void        apply_wb_to_bayer_img( img::img_descriptor& dest, byte wb_r, byte wb_gr, byte wb_b, byte wb_gb, unsigned int cpu_features );
};

#endif // BY8_APPLY_WHITEBALANCE_H_INC_
