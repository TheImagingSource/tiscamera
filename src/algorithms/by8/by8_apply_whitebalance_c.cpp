

#include "by8_apply_whitebalance.h"
#include "by8_base.h"
#include "../img/cpu_features.h"

namespace {

FORCEINLINE uint16_t wb_pixel_c_16bit( uint16_t pixel, byte factor )
{
	int val = (pixel * factor) / 64;
	return  val > 0xFFFF ? 0xFFFF : uint16_t(val);
}

FORCEINLINE byte wb_pixel_c_8bit( byte pixel, byte factor )
{
	int val = (pixel * factor) / 64;
	return  val > 0xFF ? 0xFF : byte( val );
}

static void wb_line_c_8bit( byte* dest_line, byte* src_line, unsigned int dim_x, byte factor_00, byte factor_01 )
{
	unsigned int x = 0;
	for( ; x < dim_x; x += 2 )
	{
		unsigned int v0 = wb_pixel_c_8bit( src_line[x], factor_00 );
		unsigned int v1 = wb_pixel_c_8bit( src_line[x + 1], factor_01 );
		*((uint16_t*)(dest_line + x)) = (uint16_t)(v1 << 8 | v0);
	}
	if( x == (dim_x - 1) )
	{
		dest_line[x] = wb_pixel_c_8bit( src_line[x], factor_00 );
	}
}

static void wb_line_c_16bit( uint16_t* dest_line, uint16_t* src_line, unsigned int dim_x, byte factor_00, byte factor_01 )
{
	unsigned int x = 0;
	for( ; x < dim_x; x += 2 )
	{
		unsigned int v0 = wb_pixel_c_16bit( src_line[x], factor_00 );
		unsigned int v1 = wb_pixel_c_16bit( src_line[x + 1], factor_01 );
		*((uint32_t*)(dest_line + x)) = (uint32_t)(v1 << 16 | v0);
	}
	if( x == (dim_x - 1) )
	{
		dest_line[x] = wb_pixel_c_16bit( src_line[x], factor_00 );
	}
}

static void	wb_image_c_8bit( img::img_descriptor& dest, const img::img_descriptor& src, byte factor_00, byte factor_01, byte factor_10, byte factor_11 )
{
	unsigned int y = 0;
	for( ; y < (dest.dim_y - 1); y += 2 )
	{
		byte* src_line0 = src.pData + y * src.pitch;
		byte* dest_line0 = dest.pData + y * dest.pitch;
		byte* src_line1 = src.pData + (y + 1) * src.pitch;
		byte* dest_line1 = dest.pData + (y + 1) * dest.pitch;

		wb_line_c_8bit( dest_line0, src_line0, dest.dim_x, factor_00, factor_01 );		// even line
		wb_line_c_8bit( dest_line1, src_line1, dest.dim_x, factor_10, factor_11 );		// odd line
	}
	if( y == (dest.dim_y - 1) )
	{
		byte* src_line0 = src.pData + y * src.pitch;
		byte* dest_line0 = dest.pData + y * dest.pitch;

		wb_line_c_8bit( dest_line0, src_line0, dest.dim_x, factor_00, factor_01 );
	}
}

static void	wb_image_c_8bit( img::img_descriptor& dest, byte factor_00, byte factor_01, byte factor_10, byte factor_11 )
{
	unsigned int y = 0;
	for( ; y < (dest.dim_y - 1); y += 2 )
	{
		byte* dest_line0 = dest.pData + y * dest.pitch;
		byte* dest_line1 = dest.pData + (y + 1) * dest.pitch;

		wb_line_c_8bit( dest_line0, dest_line0, dest.dim_x, factor_00, factor_01 );		// even line
		wb_line_c_8bit( dest_line1, dest_line1, dest.dim_x, factor_10, factor_11 );		// odd line
	}
	if( y == (dest.dim_y - 1) )
	{
		byte* dest_line0 = dest.pData + y * dest.pitch;

		wb_line_c_8bit( dest_line0, dest_line0, dest.dim_x, factor_00, factor_01 );
	}
}

static void	wb_image_c_16bit( img::img_descriptor& dest, byte factor_00, byte factor_01, byte factor_10, byte factor_11 )
{
	unsigned int y = 0;
	for( ; y < (dest.dim_y - 1); y += 2 )
	{
		uint16_t* dest_line0 = reinterpret_cast<uint16_t*>( dest.pData + y * dest.pitch );
		uint16_t* dest_line1 = reinterpret_cast<uint16_t*>( dest.pData + (y + 1) * dest.pitch );

		wb_line_c_16bit( dest_line0, dest_line0, dest.dim_x, factor_00, factor_01 );		// even line
		wb_line_c_16bit( dest_line1, dest_line1, dest.dim_x, factor_10, factor_11 );		// odd line
	}
	if( y == (dest.dim_y - 1) )
	{
		uint16_t* dest_line0 = reinterpret_cast<uint16_t*>(dest.pData + y * dest.pitch);

		wb_line_c_16bit( dest_line0, dest_line0, dest.dim_x, factor_00, factor_01 );
	}
}

static void	wb_image_c_16bit( img::img_descriptor& dest, const img::img_descriptor& src, byte factor_00, byte factor_01, byte factor_10, byte factor_11 )
{
	unsigned int y = 0;
	for( ; y < (dest.dim_y - 1); y += 2 )
	{
		uint16_t* src_line0 = reinterpret_cast<uint16_t*>( src.pData + y * src.pitch );
		uint16_t* dest_line0 = reinterpret_cast<uint16_t*>( dest.pData + y * dest.pitch );
		uint16_t* src_line1 = reinterpret_cast<uint16_t*>( src.pData + (y + 1) * src.pitch );
		uint16_t* dest_line1 = reinterpret_cast<uint16_t*>( dest.pData + (y + 1) * dest.pitch );

		wb_line_c_16bit( dest_line0, src_line0, dest.dim_x, factor_00, factor_01 );		// even line
		wb_line_c_16bit( dest_line1, src_line1, dest.dim_x, factor_10, factor_11 );		// odd line
	}
	if( y == (dest.dim_y - 1) )
	{
		uint16_t* src_line0 = reinterpret_cast<uint16_t*>(src.pData + y * src.pitch);
		uint16_t* dest_line0 = reinterpret_cast<uint16_t*>(dest.pData + y * dest.pitch);

		wb_line_c_16bit( dest_line0, src_line0, dest.dim_x, factor_00, factor_01 );
	}
}


}

void		by8_transform::apply_wb_to_bayer_img_c( img::img_descriptor& data, byte wb_r, byte wb_g, byte wb_b )
{
	return apply_wb_to_bayer_img_c(data, wb_r, wb_g, wb_b, wb_g);
}

void		by8_transform::apply_wb_to_bayer_img_c( img::img_descriptor& data, byte wb_r, byte wb_gr, byte wb_b, byte wb_gb )
{
	ASSERT( data.pitch );

	switch( data.type )
	{
	case FOURCC_BGGR8:	wb_image_c_8bit( data, wb_b, wb_gb, wb_gr, wb_r ); break;
	case FOURCC_GBRG8:	wb_image_c_8bit( data, wb_gb, wb_b, wb_r, wb_gr ); break;
	case FOURCC_GRBG8:	wb_image_c_8bit( data, wb_gr, wb_r, wb_b, wb_gb ); break;
	case FOURCC_RGGB8:	wb_image_c_8bit( data, wb_r, wb_gr, wb_gb, wb_b ); break;

	case FOURCC_BGGR16:	wb_image_c_16bit( data, wb_b, wb_gb, wb_gr, wb_r ); break;
	case FOURCC_GBRG16:	wb_image_c_16bit( data, wb_gb, wb_b, wb_r, wb_gr ); break;
	case FOURCC_GRBG16:	wb_image_c_16bit( data, wb_gr, wb_r, wb_b, wb_gb ); break;
	case FOURCC_RGGB16:	wb_image_c_16bit( data, wb_r, wb_gr, wb_gb, wb_b ); break;
	};
}

void		by8_transform::apply_wb_to_bayer_img_c( img::img_descriptor& dest, const img::img_descriptor& src, byte wb_r, byte wb_g, byte wb_b )
{
	return apply_wb_to_bayer_img_c(dest, src, wb_r, wb_g, wb_b, wb_g);
}

void		by8_transform::apply_wb_to_bayer_img_c( img::img_descriptor& dest, const img::img_descriptor& src, byte wb_r, byte wb_gr, byte wb_b, byte wb_gb )
{
	ASSERT( dest.pitch && src.pitch );

	switch( src.type )
	{
	case FOURCC_BGGR8:	wb_image_c_8bit( dest, src, wb_b, wb_gb, wb_gr, wb_r ); break;
	case FOURCC_GBRG8:	wb_image_c_8bit( dest, src, wb_gb, wb_b, wb_r, wb_gr ); break;
	case FOURCC_GRBG8:	wb_image_c_8bit( dest, src, wb_gr, wb_r, wb_b, wb_gb ); break;
	case FOURCC_RGGB8:	wb_image_c_8bit( dest, src, wb_r, wb_gr, wb_gb, wb_b ); break;

	case FOURCC_BGGR16:	wb_image_c_16bit( dest, src, wb_b, wb_gb, wb_gr, wb_r ); break;
	case FOURCC_GBRG16:	wb_image_c_16bit( dest, src, wb_gb, wb_b, wb_r, wb_gr ); break;
	case FOURCC_GRBG16:	wb_image_c_16bit( dest, src, wb_gr, wb_r, wb_b, wb_gb ); break;
	case FOURCC_RGGB16:	wb_image_c_16bit( dest, src, wb_r, wb_gr, wb_gb, wb_b ); break;
	};
}

void        by8_transform::apply_wb_to_bayer_img( img::img_descriptor& dest, byte wb_r, byte wb_gr, byte wb_b, byte wb_gb, unsigned int cpu_features )
{
        apply_wb_to_bayer_img_c( dest, wb_r, wb_gr, wb_b, wb_gb );

}
