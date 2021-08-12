

#include "fcc1x_packed_to_fcc.h"

#include "../../simd_helper/use_simd_A64.h"

#include "fcc1x_packed_to_fcc8_internal_loop.h"

using namespace fcc1x_packed_internal;


namespace
{

void transform_fcc12_packed_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim.cx % 2 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );

        int x = 0;
        for( int src_offset = 0; x < (dst.dim.cx - 15); x += 16, src_offset += 24 )
        {
            uint8x8x3_t vals = vld3_u8( src_line + src_offset );                               // vals[0] u8-bits = u8{p0[4:B]        }, u8{p2[4:B]        }, u8{p4[4:B]} ...

            uint8x8x2_t res = { vals.val[0], vals.val[2] };
            vst2_u8( dst_line + x, res );
        }
        
        transform_fcc12_packed_to_fcc8_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc12_mipi_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim.cx % 2 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );

        int x = 0;
        for( int src_offset = 0; x < (dst.dim.cx - 15); x += 16, src_offset += 24 )
        {
#if 1
            uint8x8x3_t vals = vld3_u8( src_line + src_offset );                    // vals[0] u8-bits = u8{p0[4:B]        }, u8{p2[4:B]        }, u8{p4[4:B]} ...
                                                                                    // vals[1] u8-bits = u8{p1[4:B]        }, u8{p3[4:B]        }, u8{p5[4:B]},...
                                                                                    // vals[2] u8-bits = u8{p0[0:3],p1[0:3]}, u8{p2[0:3],p3[0:3]}, 
			uint8x8x2_t res = { vals.val[0], vals.val[1] };
            vst2_u8( dst_line + x, res );
#else
            // this is about as fast as the upper version, but uses larger steps, so we just use the upper version
            uint8x16x3_t vals = vld3q_u8( src_line + (x / 2) * 3 );                   // vals[0] u8-bits = u8{p0[4:B]        }, u8{p2[4:B]        }, u8{p4[4:B]} ...
                                                                                    // vals[1] u8-bits = u8{p1[4:B]        }, u8{p3[4:B]        }, u8{p5[4:B]},...
                                                                                    // vals[2] u8-bits = u8{p0[0:3],p1[0:3]}, u8{p2[0:3],p3[0:3]}, 
            uint8x16x2_t res = { vals.val[0], vals.val[1] };
            vst2q_u8( dst_line + x, res );
#endif
        }

        transform_fcc12_mipi_to_fcc8_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc12_spacked_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim.cx % 2 == 0 );

    // u8 11111111'11110000'00000000

    //val |= ((uint32_t( src_ptr[0] & 0xFF ) << 4) | (uint32_t( src_ptr[1] & 0x0F ) << 12)) << 0;
    //val |= ((uint32_t( src_ptr[1] & 0xF0 ) << 0) | (uint32_t( src_ptr[2] & 0xFF ) << 8)) << 16;

    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 15); x += 16 )
        {
			// v0 = FEDC'BA98'7654'3210
            // p0 =[21], p1 = [54]

            uint8x8x3_t vals = vld3_u8( src_line + (x / 2) * 3 );   // vals[0] = u8{p0[0:7]        }, u8{p2[0:7]        }, u8{p4[0:7]        } ...
                                                                    // vals[1] = u8{p0[8:B],p1[0,3]}, u8{p2[8:B],p3[0,3]}, u8{p4[8:B],p5[0,3]} ...
                                                                    // vals[2] = u8{p1[4:B]        }, u8{p3[4:B]        }, u8{p5[4:B]        } ...

            uint8x8x2_t tmp0 = vzip_u8( vals.val[0], vals.val[1] );
            uint16x4_t tmp1 = vreinterpret_u16_u8( tmp0.val[0] );
			uint16x4_t tmp2 = vreinterpret_u16_u8( tmp0.val[1] );

            uint16x8_t tmp3 = vcombine_u16( tmp1, tmp2 );
            uint16x8_t tmp4 = vshlq_n_u16( tmp3, 4 );
			uint16x8_t tmp5 = vshrq_n_u16( tmp4, 8 );

            uint8x8_t tmp6 = vqmovn_u16( tmp5 );

            uint8x8x2_t res = { tmp6, vals.val[2] };
            vst2_u8( dst_line + x, res );
        }

        transform_fcc12_spacked_to_fcc8_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc10_spacked_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim.cx % 4 == 0 );

    // src = u8[] = XXc2c2c2c2c2c1c1'c1c1c1c0c0c0c0c0       (c denotes cluster)

    //  cluster, bits = 33333333'33222222'22221111'11111100'00000000

    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint8_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );

        for( int x = 0; x < src.dim.cx; x += 4 )
        {
			const uint8_t* src_ptr = src_line + (x / 4) * 5;
			dst_line[x + 0] = ((src_ptr[0] & 0b11111100) >> 2) | ((src_ptr[1] & 0b00000011) << 6);
			dst_line[x + 1] = ((src_ptr[1] & 0b11110000) >> 4) | ((src_ptr[2] & 0b00001111) << 4);
			dst_line[x + 2] = ((src_ptr[2] & 0b11000000) >> 6) | ((src_ptr[3] & 0b00111111) << 2);
			dst_line[x + 3] = src_ptr[4];
        }
    }
}

void transform_fcc10_mipi_to_fcc8_neon_v0(img::img_descriptor dst, img::img_descriptor src)
{
    assert( src.dim.cx % 4 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto srcPtr = img::get_line_start<const uint8_t>( src, y );
        auto dstPtr = img::get_line_start<uint8_t>( dst, y );

        auto dstLineEnd = dstPtr + dst.dim.cx;
        while( dstPtr < dstLineEnd )
        {
            // copy four bytes, which is exactly the four upper 8 bits of pixels 0-3
            *reinterpret_cast<uint32_t*>(dstPtr) = *reinterpret_cast<const uint32_t*>(srcPtr);
            srcPtr += 5;
            dstPtr += 4;
        }
    }
}

void    transform_fcc12_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );
        int x = 0;
        for( ; x < (src.dim.cx - 7); x += 8 )
        {
            const uint16x8_t vals = vld1q_u16( src_line + x );
            const uint16x8_t tmp0 = vshrq_n_u16( vals, 4 );
            const uint8x8_t res = vmovn_u16( tmp0 );

            vst1_u8( dst_line + x, res );
        }
        for( ; x < src.dim.cx; ++x )
        {
            dst_line[x] = calc_fcc12_to_fcc8( src_line, x );
        }
    }
}

void    transform_fcc10_to_fcc8_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<uint8_t>( dst, y );
        int x = 0;
        for( ; x < (src.dim.cx - 7); x += 8 )
        {
            const uint16x8_t vals = vld1q_u16( src_line + x );
            const uint16x8_t tmp0 = vshrq_n_u16( vals, 2 );
            const uint8x8_t res = vmovn_u16( tmp0 );

            vst1_u8( dst_line + x, res );
        }
        for( ; x < src.dim.cx; ++x )
        {
            dst_line[x] = calc_fcc10_to_fcc8( src_line, x );
        }
    }
}

}

auto img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_neon_v0( const img::img_type& dst, const img::img_type& src ) 
-> img_filter::transform_function_type
{
    if( dst.dim != src.dim ) {
        return nullptr;
    }

    using namespace img::fcc1x_packed;

    if( !is_accepted_dst_fcc8( dst.fourcc_type() ) ) {
        return nullptr;
    }

    switch( get_fcc1x_pack_type( src.fourcc_type() ) )
    {
    case fccXX_pack_type::fcc12:                return ::transform_fcc12_to_fcc8_neon_v0;
    case fccXX_pack_type::fcc12_mipi:           return ::transform_fcc12_mipi_to_fcc8_neon_v0;
    case fccXX_pack_type::fcc12_packed:         return ::transform_fcc12_packed_to_fcc8_neon_v0;
    case fccXX_pack_type::fcc12_spacked:        return ::transform_fcc12_spacked_to_fcc8_neon_v0;

    case fccXX_pack_type::fcc10:                return ::transform_fcc10_to_fcc8_neon_v0;
    case fccXX_pack_type::fcc10_spacked:        return ::transform_fcc10_spacked_to_fcc8_neon_v0;
    case fccXX_pack_type::fcc10_mipi:           return ::transform_fcc10_mipi_to_fcc8_neon_v0;

    case fccXX_pack_type::invalid:              return nullptr;
    };
    return nullptr;
}