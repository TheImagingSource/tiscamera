

#include "auto_alg.h"

using namespace auto_alg;

#define SAMPLING_LINES		30
#define SAMPLING_COLUMNS	40


void auto_alg::auto_sample_by8img( const img::img_descriptor& image, auto_sample_points& points )
{
	ASSERT( image.dim_x != 0 && image.dim_y != 0 && image.pitch != 0 );

	unsigned int first_line_offset = 0;
	// offset the start pointer to convert to a GB start pattern
	switch( by8_transform::convertFCCToPattern( image.type ) )
	{
	case by8_transform::GB: first_line_offset += 0;					break;
	case by8_transform::BG: first_line_offset += 1;					break;
	case by8_transform::RG: first_line_offset += image.pitch;		break;
	case by8_transform::GR: first_line_offset += image.pitch + 1;	break;
	};

    if( image.dim_x < 3 || image.dim_y < 3 )
        return;

	unsigned int x_step = image.dim_x / (SAMPLING_COLUMNS + 1);
	unsigned int y_step = image.dim_y / (SAMPLING_LINES + 1);
	if( y_step == 0 || x_step == 0 )
		return;

    unsigned max_count = (sizeof( points.samples ) / sizeof( points.samples[0] ));

	unsigned int cnt = 0;	// max is 42 * 32
	for( unsigned int y = y_step; y < (image.dim_y - 2); y += y_step )
	{
		byte* pLine = image.pData + first_line_offset + y * image.pitch;
		byte* pNextLine = pLine + image.pitch;

		for( unsigned int col = x_step; col < (image.dim_x - 2); col += x_step )
		{
			unsigned int r = 0, gr = 0, b = 0, gb = 0;
			if( y & 1 )
			{
				if( col & 1 )
				{
					r = pLine[col+1];
					gr = pLine[col];
					b = pNextLine[col];
					gb = pNextLine[col+1];
				}
				else
				{
					r = pLine[col];
					gr = pLine[col+1];
					b = pNextLine[col+1];
					gb = pNextLine[col];
				}
			}
			else
			{
				if( col & 1 )
				{
					r = pNextLine[col+1];
					gb = pLine[col+1];
					b = pLine[col];
					gr = pNextLine[col];
				}
				else
				{
					r = pNextLine[col];
					gb = pLine[col];
					b = pLine[col+1];
					gr = pNextLine[col+1];
				}
			}
			if( cnt >= max_count )
                break;

            points.samples[cnt].r = (byte) r;
			points.samples[cnt].gr = (byte) gr;
			points.samples[cnt].b = (byte) b;
			points.samples[cnt].gb = (byte) gb;
			points.samples[cnt].g = (byte)((gr + gb) / 2);
			++cnt;
		}
	}
	points.cnt = cnt;
}


void auto_alg::auto_sample_by_img( const img::img_descriptor& image, auto_sample_points& points )
{
    points.cnt = 0;

    ASSERT( img::isBayerFCC( image.type ) || img::isBayer16FCC( image.type ) );
    if( img::isBayerFCC( image.type ) )
        auto_sample_by8img( image, points );
}


void	auto_alg::auto_sample_y8img( const img::img_descriptor& image, int& brightness )
{
	ASSERT( image.dim_x != 0 && image.dim_y != 0 && image.pitch != 0 );

	brightness = 128;

	unsigned int y_step = image.dim_y / (SAMPLING_LINES+1);
	unsigned int x_step = image.dim_x / (SAMPLING_COLUMNS+1);
	if( y_step == 0 || x_step == 0 )
		return;

	unsigned int cnt = 0;
	unsigned int pixel_accu = 0;
	for( unsigned int y = y_step; y < image.dim_y; y += y_step )
	{
		byte* line = image.pData + y * image.pitch;
		for( unsigned int col = x_step; col < image.dim_x; col += x_step )
		{
			++cnt;
			pixel_accu += line[col];
		}
	}
	if( cnt )
	{
		brightness = pixel_accu / cnt;
	}
}


void	auto_alg::auto_sample_mono_img( const img::img_descriptor& image, int& brightness, float& factor_y_vgt240 )
{
    if( image.type == FOURCC_Y800 )
    {
        auto_sample_y8img( image, brightness, factor_y_vgt240 );
    }
}


void	auto_alg::auto_sample_y8img( const img::img_descriptor& image, int& brightness, float& factor_y_vgt240 )
{
	ASSERT( image.dim_x != 0 && image.dim_y != 0 && image.pitch != 0 );

	brightness = 128;

	unsigned int y_step = image.dim_y / (SAMPLING_LINES + 1);
	unsigned int x_step = image.dim_x / (SAMPLING_COLUMNS + 1);
	if( y_step == 0 || x_step == 0 )
		return;

	unsigned int cnt = 0;
	unsigned int pixel_accu = 0;
	unsigned int cnt_y_gt_240 = 0;
	for( unsigned int y = y_step; y < image.dim_y; y += y_step )
	{
		byte* line = image.pData + y * image.pitch;
		for( unsigned int x = x_step; x < image.dim_x; x += x_step )
		{
			++cnt;
			pixel_accu += line[x];
			if( line[x] >= 240 )
				++cnt_y_gt_240;
		}
	}
	if( cnt )
	{
		brightness = pixel_accu / cnt;
		factor_y_vgt240 = cnt_y_gt_240 / (float)cnt;
	}
}


unsigned int	auto_alg::calc_resulting_brightness( const auto_alg::auto_sample_points& points )
{
	if( points.cnt == 0 )
		return 128;

	int accu_r = 0, accu_g = 0, accu_b = 0;
	for( int i = 0; i < points.cnt; ++i )
	{
		int r = points.samples[i].r;
		int g = points.samples[i].g;
		int b = points.samples[i].b;

		//apply_color_matrix_c( clr, r, g, b );

		accu_r += r;
		accu_g += g;
		accu_b += b;
	}
	return calc_brightness_from_clr_avg( accu_r / points.cnt, accu_g / points.cnt, accu_b / points.cnt );
}


unsigned int	auto_alg::calc_resulting_brightness( const auto_alg::auto_sample_points& points, const by8_mtx& clr )
{
	if( points.cnt == 0 )
		return 128;

	int accu_r = 0, accu_g = 0, accu_b = 0;
	for( int i = 0; i < points.cnt; ++i )
	{
		int r = points.samples[i].r;
		int g = points.samples[i].g;
		int b = points.samples[i].b;

		accu_r += r;
		accu_g += g;
		accu_b += b;
	}
	return calc_brightness_from_clr_avg( accu_r / points.cnt, accu_g / points.cnt, accu_b / points.cnt );
}


void		auto_alg::calc_resulting_brightness_params( int& brightness, float& factor_y_vgt240, const auto_alg::auto_sample_points& points )
{
    brightness = 128;
    factor_y_vgt240 = -1.0f;

    if( points.cnt <= 0 )
        return;

    int counter = 0;
    int brightness_accu = 0;
    for( int idx = 0; idx < points.cnt; ++idx )
    {
        int r = points.samples[idx].r;
        int g = points.samples[idx].g;
        int b = points.samples[idx].b;

        int y = auto_alg::calc_brightness_from_clr_avg( r, g, b );
        if( y >= 240 )
            ++counter;

        brightness_accu += y;
    }

    factor_y_vgt240 = counter / (float)points.cnt;
    brightness = brightness_accu / points.cnt;
}


void		auto_alg::calc_resulting_brightness_params( int& brightness, float& factor_y_vgt240, const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx )
{
    if( !clr_mtx.enabled )
        return calc_resulting_brightness_params( brightness, factor_y_vgt240, points );

    brightness = 128;
    factor_y_vgt240 = -1.0f;

    if( points.cnt <= 0 )
        return;

    int counter = 0;
    int brightness_accu = 0;
    for( int idx = 0; idx < points.cnt; ++idx )
    {
        int r = points.samples[idx].r;
        int g = points.samples[idx].g;
        int b = points.samples[idx].b;

        int y = auto_alg::calc_brightness_from_clr_avg( r, g, b );
        if( y >= 240 )
            ++counter;

        brightness_accu += y;
    }

    factor_y_vgt240 = counter / (float)points.cnt;
    brightness = brightness_accu / points.cnt;
}


void		auto_alg::calc_resulting_brightness_params( int& brightness, float& factor_y_vgt240, const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx, const auto_alg::rgb_tripel& wb_params )
{
    brightness = 128;
    factor_y_vgt240 = -1.0f;

    if( points.cnt <= 0 )
        return;

    int counter = 0;
    int brightness_accu = 0;
    for( int idx = 0; idx < points.cnt; ++idx )
    {
        int r = points.samples[idx].r;
        int g = points.samples[idx].g;
        int b = points.samples[idx].b;

        r = r * wb_params.r / 64;
        g = g * wb_params.g / 64;
        b = b * wb_params.b / 64;

        int y = auto_alg::calc_brightness_from_clr_avg( r, g, b );
        if( y >= 240 )
            ++counter;

        brightness_accu += y;
    }

    factor_y_vgt240 = counter / (float)points.cnt;
    brightness = brightness_accu / points.cnt;
}


float		auto_alg::calc_adjusted_y_vgt240( const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx )
{
    if( points.cnt <= 0 )
        return -1.0f;

    int counter = 0;
    for( int idx = 0; idx < points.cnt; ++idx )
    {
        int r = points.samples[idx].r;
        int g = points.samples[idx].g;
        int b = points.samples[idx].b;

        int y = auto_alg::calc_brightness_from_clr_avg( r, g, b );
        if( y >= 240 )
            ++counter;
    }
    return counter / (float)points.cnt;
}


float		auto_alg::calc_adjusted_y_vgt240( const auto_alg::auto_sample_points& points, const auto_alg::by8_mtx& clr_mtx, const auto_alg::rgb_tripel& wb_params )
{
    if( points.cnt <= 0 )
        return -1.0f;

    int counter = 0;
    for( int idx = 0; idx < points.cnt; ++idx )
    {
        int r = points.samples[idx].r;
        int g = points.samples[idx].g;
        int b = points.samples[idx].b;

        r = r * wb_params.r / 64;
        g = g * wb_params.g / 64;
        b = b * wb_params.b / 64;

        int y = auto_alg::calc_brightness_from_clr_avg( CLIP( r, 0, 0xFF ), CLIP( g, 0, 0xFF ), CLIP( b, 0, 0xFF ) );
        if( y >= 240 )
            ++counter;
    }
    return counter / (float)points.cnt;
}
