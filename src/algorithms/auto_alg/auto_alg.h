
#ifndef AUTO_ALG_H_INC_
#define AUTO_ALG_H_INC_

#include "../image_transform_interop.h"

#include "pid_controller.h"
#include "auto_alg_params.h"

#include "auto_exposure.h"

namespace auto_alg
{
	struct auto_sample_points
	{
		struct pixel
		{
			byte	r, gr, b, gb;
			byte	g;
		} samples[2000];
		int	cnt;
	};

	struct rgb_tripel
	{
		int r, g, b;
	};

	typedef color_matrix_params by8_mtx;

	bool		auto_whitebalance( const auto_sample_points& data, const by8_mtx& clr, rgb_tripel& wb );
	bool		auto_whitebalance_cam( const auto_sample_points& data, const by8_mtx& mtx, rgb_tripel& wb );

	// Sampling functions
	void			auto_sample_by8img( const img::img_descriptor& image, auto_sample_points& points );
    void            auto_sample_by_img( const img::img_descriptor& image, auto_sample_points& points );

	unsigned int	calc_resulting_brightness( const auto_sample_points& samples );
	unsigned int	calc_resulting_brightness( const auto_sample_points& samples, const by8_mtx& clr );
	unsigned int	calc_resulting_brightness( const auto_sample_points& samples, const by8_mtx& clr, const rgb_tripel& wb );


    void            auto_sample_mono_img( const img::img_descriptor& image, int& brightness, float& factor_y_vgt240 );


	void			auto_sample_y8img( const img::img_descriptor& image, int& brightness );

	void			auto_sample_y8img( const img::img_descriptor& image, int& brightness, float& factor_y_vgt240 );

	static const unsigned int r_factor = (unsigned int)((1 << 8) * 0.299f);
	static const unsigned int g_factor = (unsigned int)((1 << 8) * 0.587f);
	static const unsigned int b_factor = (unsigned int)((1 << 8) * 0.114f);

	inline unsigned int	calc_brightness_from_clr_avg( unsigned int r, unsigned int g, unsigned int b )
	{
		return (r * r_factor + g * g_factor + b * b_factor) >> 8;
	}

	inline unsigned int	calc_brightness_from_clr_avg( const auto_alg::rgb_tripel& vals )
	{
		return calc_brightness_from_clr_avg( vals.r, vals.g, vals.b );
	}

    float   calc_adjusted_y_vgt240( const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx );
    float   calc_adjusted_y_vgt240( const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx, const auto_alg::rgb_tripel& wb_params );

    void	calc_resulting_brightness_params( int& brightness, float& factor_y_vgt240, const auto_alg::auto_sample_points& points );
    void    calc_resulting_brightness_params( int& brightness, float& factor_y_vgt240, const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx );
    void    calc_resulting_brightness_params( int& brightness, float& factor_y_vgt240, const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx, const auto_alg::rgb_tripel& wb_params );
};

#endif // AUTO_ALG_H_INC_
