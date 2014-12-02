
// #include "stdafx.h"

#include "img_transform_pipe.h"
#include "../by8/by8torgb_conv.h"
#include "../yuv/yuv_planar_transform_impl.h"
#include "../filter/smart_sharpening.h"
#include "../yuv/yuv_planar_apply_params.h"
#include "../filter/denoise.h"
#include "../filter/transform_empiay8_to_rgb.h"
#include "../filter/transform_y8_to_rgb.h"
#include "../filter/transform_empiay8_to_y8.h"
#include "../filter/filter_lut.h"

#include "../img/memcpy_image.h"

#include "../by8/by8_transform_matrix.h"

#include "..//filter/sharpness_denoise.h"


#if KERNEL_DRIVER_
namespace std
{
	template<class T>
	void	swap( T& t0, T& t1 )
	{
		T tmp = t0;
		t0 = t1;
		t1 = tmp;
	}
};
#else
#include <algorithm>
#endif
#include "../by8/by8_apply_whitebalance.h"
#include "../by16/by16_transform_matrix.h"
#include "../yuv/yuv16_transform.h"
#include "../yuv/yuy2_transform.h"

#undef PROFILER_ENABLE_

#if !defined _DEBUG
const unsigned int fudge_factor = 4096;     // outside of debug mode, we add a couple of bytes to workaround difficult to see overreaching crashes
#else
const unsigned int fudge_factor = 0;
#endif

int	    img::get_needed_scratchspace_size( const img_type& dst_img, const img_type& src_img )
{
	if( img::isBayerFCC( src_img.type ) && (dst_img.type == FOURCC_RGB32 || dst_img.type == FOURCC_RGB24) )
	{
        return fudge_factor + calc_minimum_img_size( FOURCC_YUV8PLANAR, dst_img.dim_x, dst_img.dim_y );
	}
	else if( src_img.type == FOURCC_Y800 && (dst_img.type == FOURCC_Y800 || dst_img.type == FOURCC_RGB24) )
	{
        return fudge_factor + calc_minimum_img_size( FOURCC_Y800, dst_img.dim_x, dst_img.dim_y );
	}
    else if( dst_img.type == FOURCC_Y16 )      // transform_y16_to_y16 maybe called
    {
        return fudge_factor + calc_minimum_img_size( FOURCC_Y16, dst_img.dim_x, dst_img.dim_y );
    }
    else if( img::isBayer16FCC( src_img.type ) &&
             (dst_img.type == FOURCC_RGB32 || dst_img.type == FOURCC_YUV16PLANAR || dst_img.type == FOURCC_RGB64) )
    {
        return fudge_factor + calc_minimum_img_size( FOURCC_YUV16PLANAR, dst_img.dim_x, dst_img.dim_y );
    }
    else if( src_img.type == FOURCC_YUY2 && dst_img.type == FOURCC_YUY2 )
    {
        // we need 2 images to allow us to get a temporary copy
        return fudge_factor + calc_minimum_img_size( FOURCC_YUV8PLANAR, dst_img.dim_x, dst_img.dim_y ) * 2;
    }
	return 0;
}


struct intermediate_copy_buffer
{
	intermediate_copy_buffer( bool odd_numbered_copies, byte* dst, byte* tmp )
		: dst_buf( dst ), tmp_buf( tmp )
	{
		return_dst_ = odd_numbered_copies;
	}

	bool	return_dst_;

	byte*	get_next_ptr()
	{
		return_dst_ = !return_dst_;
		if( !return_dst_ )
			return dst_buf;
		return tmp_buf;
	}

	byte*	tmp_buf;
	byte*	dst_buf;
};


static void transform_image_by8_to_rgb( img::img_descriptor& dest_img,
                                        img::img_descriptor& src_img,
                                        const img::img_transform_params& params );
static void transform_image_by16_to_rgb( img::img_descriptor& dest_img,
                                         img::img_descriptor& src_img,
                                         const img::img_transform_params& params );
static void transform_image_yuy2_to_yuy2( img::img_descriptor& dst_img,
                                          img::img_descriptor& src_img,
                                          const img::img_transform_params& params );


static void transform_image_y800_to_y800( img::img_descriptor& dest_img,
                                          img::img_descriptor& src_img,
                                          const img::img_transform_params& params )
{
	if( params.enable_green_pattern_fix )
	{ // Y800 to Y800 and enable_green_pattern_fix == true => mpiay8_to_y8
		img_transform::apply_empia_fix( src_img, params.scratch_space, params.scratch_space_len, params.cpu_features );
	}

    if( params.lut.use_lut )
    {
        img::lut::apply_y8( src_img, params.lut.table_y8 );
    }
    else
    {
        img::yuv::apply_y_params( src_img, params.brightness, params.contrast, params.cpu_features );
    }

    if( params.denoise_amount != 0 || params.sharpness != 0 )
	{
        img::sharpness_denoise::apply( dest_img, src_img, params.sharpness, params.denoise_amount, params.cpu_features );
	}
    else
    {
		img::memcpy_image( dest_img, src_img, params.flip_h, params.cpu_features );
	}
}


static void transform_image_y16_to_y16( img::img_descriptor& dst_image, img::img_descriptor& src_img, const img::img_transform_params& params )
{
    if( params.enable_green_pattern_fix )
    {
        img_transform::apply_empia_fix( src_img, params.scratch_space, params.scratch_space_len, params.cpu_features );
    }

    if( params.lut.use_lut )
    {
        img::lut::apply_y16( src_img, params.lut.table_y16 );
    }
    else
    {
        img::yuv::apply_y_params( src_img, params.brightness, params.contrast, params.cpu_features );
    }

    if( params.denoise_amount != 0 || params.sharpness != 0 )
    {
        img::sharpness_denoise::apply( dst_image, src_img, params.sharpness, params.denoise_amount, params.cpu_features );
    }
    else
    {
        img::memcpy_image( dst_image, src_img, params.flip_h, params.cpu_features );
    }
}


static void transform_image_y800_to_rgb24( img::img_descriptor& dest_img, img::img_descriptor& src_img, const img::img_transform_params& params )
{
	// if we need to call memcpy_image
	bool odd_transform_step_count = true ^ ((params.sharpness != 0) || (params.denoise_amount != 0));

	intermediate_copy_buffer int_buf( odd_transform_step_count, dest_img.pData, (byte*)params.scratch_space );

	img::img_type temp_img_type = { FOURCC_Y800, src_img.dim_x, src_img.dim_y, src_img.dim_x, src_img.dim_x * src_img.dim_y };

	byte* src_ptr = src_img.pData;
	byte* dst_ptr = int_buf.get_next_ptr();

	if( params.enable_green_pattern_fix )
	{
		img_transform::apply_empia_y8_to_y8( src_img, params.scratch_space, params.scratch_space_len, params.cpu_features );
	}

    if( params.denoise_amount != 0 || params.sharpness != 0 )
    {
        img::img_descriptor dst = to_img_desc( temp_img_type, dst_ptr );
        img::sharpness_denoise::apply( dst, to_img_desc( temp_img_type, src_ptr ), params.sharpness, params.denoise_amount, params.cpu_features );

        src_ptr = dst_ptr;
        dst_ptr = int_buf.get_next_ptr();
    }

    {
        img::img_descriptor dst = to_img_desc( temp_img_type, src_ptr );
        if( params.lut.use_lut )
        {
            img::lut::apply_y8( dst, params.lut.table_y8 );
        }
        else
        {
            img::yuv::apply_y_params( dst, params.brightness, params.contrast, params.cpu_features );
        }
    }
	img::img_descriptor src = img::to_img_desc( temp_img_type, src_ptr );

	img_transform::transform_y8_to_rgb24( dest_img, src, params.flip_h, params.cpu_features );
}

void img::transform_pipe( img_descriptor& dest_img, img_descriptor& src_img, const img_transform_params& params )
{
    if( params.wb.apply_wb && (img::isBayerFCC( src_img.type ) || img::isBayer16FCC( src_img.type )) )
    {
        by8_transform::apply_wb_to_bayer_img( src_img, (byte)params.wb.r, (byte)params.wb.gr, (byte)params.wb.b, (byte)params.wb.gb, params.cpu_features );
    }

	//PROFILER_START_SCOPED( __FUNCTION__ );
	if( dest_img.type == src_img.type )
	{
		if( dest_img.type == FOURCC_Y800 )
		{
			transform_image_y800_to_y800( dest_img, src_img, params );
		}
		else if( dest_img.type == FOURCC_Y16 )
        {
            transform_image_y16_to_y16( dest_img, src_img, params );
        }
        else if( src_img.type == FOURCC_YUY2 && dest_img.type == FOURCC_YUY2 )
        {
            transform_image_yuy2_to_yuy2( dest_img, src_img, params );
        }
        else
		{
			img::memcpy_image( dest_img, src_img, params.flip_h, params.cpu_features );
		}
	}
	else if( img::isBayerFCC( src_img.type ) && (dest_img.type == FOURCC_RGB32 || dest_img.type == FOURCC_RGB24) )
	{
		transform_image_by8_to_rgb( dest_img, src_img, params );
	}
	else if( img::isBayerFCC( src_img.type ) && (dest_img.type == FOURCC_BY8 || dest_img.type == FOURCC_Y800) )	// Bayer image to Y800
	{
		img_descriptor tmp_dest = img::copy_img_desc( dest_img, src_img.type ); // make a copy of the descriptor and change the type so the memcpy accepts this
		img::memcpy_image( tmp_dest, src_img, params.flip_h, params.cpu_features );
	}
    else if( img::isBayer16FCC( src_img.type ) && (dest_img.type == FOURCC_RGB32 || dest_img.type == FOURCC_YUV16PLANAR || dest_img.type == FOURCC_RGB64) )
    {
        transform_image_by16_to_rgb( dest_img, src_img, params );
    }
	else if( src_img.type == FOURCC_Y800 && dest_img.type == FOURCC_RGB24 )	// this is for mono cameras to allow generation of VfW bindings
	{
		transform_image_y800_to_rgb24( dest_img, src_img, params );
	}
	else
	{
		// this is the fall back, just copy anything
		size_t len = MIN( src_img.data_length, dest_img.data_length );
		memcpy( dest_img.pData, src_img.pData, len );
	}
}

static void transform_image_by8_to_rgb( img::img_descriptor& dst_img, img::img_descriptor& src_img, const img::img_transform_params& params )
{
	by8_transform::transform_by8_options opt = {
		by8_transform::transform_by8_options::NoOptions,
		params.cpu_features,
		params.by8_params.mtx
	};

    if( params.flip_h )
        opt.options |= by8_transform::transform_by8_options::FlipImage;
	if( params.by8_params.use_mtx )
		opt.options |= by8_transform::transform_by8_options::UseClrMatrix;
	if( params.enable_green_pattern_fix )
		opt.options |= by8_transform::transform_by8_options::UseAvgGreen;

	bool use_yuv8planar_transform =
		(params.saturation != 0 || params.contrast != 0 || params.sharpness != 0 || params.hue != 0 ||
		params.brightness != 0 ||
		params.denoise_amount > 0 ||
		params.lut.use_lut);

	if( !use_yuv8planar_transform )
	{
        if( !params.enable_by8_matrix_code )
		    by8_transform::transform_by8_to_dest( dst_img, src_img, opt );
        else
            by8_transform::transform_by8_to_dest_by_matrix( dst_img, src_img, opt );
    }
	else
	{
		// to reduce the intermediate buffer count, we reuse the dest_img as a temporary image buffer
		// to get the sequence right, we calc which buffer to use first here (either the buffer behind dest_img or the scratch buffer)
		// after every operation which needs a distinct source and destination buffer we swap the pointers
		bool scratch_buffer_first = !((params.sharpness != 0) || (params.denoise_amount != 0));

		byte* src_ptr = scratch_buffer_first ? (byte*)params.scratch_space : dst_img.pData;
		byte* dst_ptr = scratch_buffer_first ? dst_img.pData : static_cast<byte*>(params.scratch_space);

        img::img_type tmp_img_type = img::make_img_type( FOURCC_YUV8PLANAR, dst_img.dim_x, dst_img.dim_y );
        assert( params.scratch_space_len >= (int)tmp_img_type.buffer_length );

        {
			img::img_descriptor dst = img::to_img_desc( tmp_img_type, src_ptr );
            if( !params.enable_by8_matrix_code )
    			by8_transform::transform_by8_to_dest( dst, src_img, opt );
            else
                by8_transform::transform_by8_to_dest_by_matrix( dst, src_img, opt );
		}

		if( params.denoise_amount != 0 || params.sharpness != 0 )
		{
            img::img_descriptor dst = to_img_desc( tmp_img_type, dst_ptr );
            img::sharpness_denoise::apply( dst, to_img_desc( tmp_img_type, src_ptr ), params.sharpness, params.denoise_amount, params.cpu_features );
			std::swap( src_ptr, dst_ptr );
		}

        {
            img::img_descriptor dst = to_img_desc( tmp_img_type, src_ptr );
            img::yuv::apply_uv_params( dst, (params.saturation + 64) / 64.0f, params.hue / 180.0f, params.cpu_features );
            if( params.lut.use_lut )
                img::lut::apply_y8( dst, params.lut.table_y8 );
            else
                img::yuv::apply_y_params( dst, params.brightness, params.contrast, params.cpu_features );
        }

        img::img_descriptor tmp_src = img::to_img_desc( tmp_img_type, src_ptr );
        img::yuv::transform_YUV8planar_to_dest( dst_img, tmp_src, params.cpu_features );
	}
}


static void transform_image_by16_to_rgb( img::img_descriptor& dst_img, img::img_descriptor& src_img, const img::img_transform_params& params )
{
    by8_transform::transform_by8_options opt = {
        by8_transform::transform_by8_options::NoOptions,
        params.cpu_features,
        params.by8_params.mtx
    };

    if( params.by8_params.use_mtx )
        opt.options |= by8_transform::transform_by8_options::UseClrMatrix;
    if( params.flip_h )
        opt.options |= by8_transform::transform_by8_options::FlipImage;

    img::img_type temp_img_type = img::make_img_type( FOURCC_YUV16PLANAR, dst_img.dim_x, dst_img.dim_y );
    assert( params.scratch_space_len >= (int) temp_img_type.buffer_length );

    int buffers_copy_count = 0;
    if( (params.sharpness != 0) || (params.denoise_amount != 0) )
        ++buffers_copy_count;
    if( dst_img.type != FOURCC_YUV16PLANAR )
        ++buffers_copy_count;

    bool scratch_buffer_first = buffers_copy_count % 2 == 1;

    byte* src_ptr = scratch_buffer_first ? (byte*)params.scratch_space : dst_img.pData;
    byte* dst_ptr = scratch_buffer_first ? dst_img.pData : static_cast<byte*>(params.scratch_space);

    {
        img::img_descriptor dst = img::to_img_desc( temp_img_type, src_ptr );
        by_transform::transform_by16_to_dest( dst, src_img, opt );
    }

    if( params.denoise_amount != 0 || params.sharpness != 0 )
    {
        img::img_descriptor dst = to_img_desc( temp_img_type, dst_ptr );

        img::sharpness_denoise::apply( dst, to_img_desc( temp_img_type, src_ptr ), params.sharpness, params.denoise_amount, params.cpu_features );
        std::swap( src_ptr, dst_ptr );
    }

    {
        img::img_descriptor dst = to_img_desc( temp_img_type, src_ptr );
        img::yuv::apply_uv_params( dst, (params.saturation + 64) / 64.0f, params.hue / 180.0f, params.cpu_features );
        if( params.lut.use_lut )
            img::lut::apply_y16( dst, params.lut.table_y16 );
        else
            img::yuv::apply_y_params( dst, params.brightness, params.contrast, params.cpu_features );
    }

    if( dst_img.type == FOURCC_RGB64 )
    {
        img::img_descriptor src = to_img_desc( temp_img_type, src_ptr );
        img::yuv::transform_YUV16p_to_dst( dst_img, src, params.cpu_features );
    }
}

static void transform_image_yuy2_to_yuy2( img::img_descriptor& dst_img, img::img_descriptor& src_img, const img::img_transform_params& params )
{
    bool need_to_convert = params.brightness != 0 || params.contrast != 0 || params.saturation != 0 || params.hue != 0 || params.sharpness != 0 || params.denoise_amount != 0 || params.lut.use_lut;
    if( !need_to_convert )
    {
        img::memcpy_image( dst_img, src_img, params.flip_h, params.cpu_features );
        return;
    }

    img::img_type tmp_img_type = img::make_img_type( FOURCC_YUV8PLANAR, dst_img.dim_x, dst_img.dim_y );
    ASSERT( params.scratch_space_len >= (int)tmp_img_type.buffer_length );

    void* scratch_buffer_0 = params.scratch_space;
    void* scratch_buffer_1 = static_cast<byte*>( params.scratch_space ) + tmp_img_type.buffer_length;

    {
        img::img_descriptor dst = img::to_img_desc( tmp_img_type, scratch_buffer_0 );
        img::yuv::transform_YUY2_to_YUV8p( dst, src_img, params.cpu_features );
    }

    if( params.denoise_amount != 0 || params.sharpness != 0 )
    {
        img::img_descriptor dst = to_img_desc( tmp_img_type, scratch_buffer_1 );

        img::sharpness_denoise::apply( dst, to_img_desc( tmp_img_type, scratch_buffer_0 ), params.sharpness, params.denoise_amount, params.cpu_features );
        std::swap( scratch_buffer_1, scratch_buffer_0 );
    }

    {
        img::img_descriptor dst = to_img_desc( tmp_img_type, scratch_buffer_0 );
        img::yuv::apply_uv_params( dst, (params.saturation + 64) / 64.0f, params.hue / 180.0f, params.cpu_features );
        if( params.lut.use_lut )
            img::lut::apply_y8( dst, params.lut.table_y8 );
        else
            img::yuv::apply_y_params( dst, params.brightness, params.contrast, params.cpu_features );
    }

    // convert back from yuv8p
    {
        img::img_descriptor src = img::to_img_desc( tmp_img_type, scratch_buffer_0 );
        img::yuv::transform_YUV8p_to_YUY2( dst_img, src, params.cpu_features );
    }
}
