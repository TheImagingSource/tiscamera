

#include "fcc1x_packed_to_fcc.h"

//#include "../../simd_helper/use_simd_A64.h"
#include <arm_neon.h>

#include "fcc1x_packed_to_fcc16_internal.h"
#include "fcc1x_packed_to_fcc16_internal_loop.h"

using namespace fcc1x_packed_internal;

namespace
{

void transform_fcc12_packed_to_dst_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim.cx % 2 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint16_t* dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 24); x += 16 )
        {
            uint8x8x3_t vals = vld3_u8( src_line + (x / 2) * 3 );                               // vals[0] u8-bits = u8{p0[4:B]        }, u8{p2[4:B]        }, u8{p4[4:B]} ...
                                                                                                // vals[2] u8-bits = u8{p1[4:B]        }, u8{p3[4:B]        }, u8{p5[4:B]},...
                                                                                                // vals[1] u8-bits = u8{p0[0:3],p1[0:3]}, u8{p2[0:3],p3[0:3]}, 
            uint16x8_t low = vmovl_u8( vals.val[0] );                                           // u8-bits = u16{ p0[4:B], 0 }, u16{ p2[4:B], 0 }, u16{ p4[4:B], 0 }
            uint16x8_t hig = vmovl_u8( vals.val[2] );                                           // u8-bits = u16{ p1[4:B], 0 }, u16{ p3[4:B], 0 }, u16{ p5[4:B], 0 }
            uint16x8_t mid = vmovl_u8( vals.val[1] );                                           // u8-bits = u8{p0[0:3],p1[0:3], 0, 0 }, u8{p2[0:3],p3[0:4], 0, 0 },  ....

            uint16x8_t mid_hi = vandq_u16( mid, vdupq_n_u16( 0x00F0 ) );                        // u8-bits = u8{ 0, p1[0:3], 0, 0 }, u8{ 0, p3[0:4], 0, 0 },  ....
            uint16x8_t mid_lo = vshlq_n_u16( mid, 4 );                                          // u8-bits = u8{ 0, p0[0:3], 0, 0 }, u8{ 0, p2[0:3], 0, 0 },  ....

            auto tmp_h0 = vsliq_n_u16( mid_hi, hig, 8 );
            auto tmp_l0 = vsliq_n_u16( mid_lo, low, 8 );

            uint16x8x2_t res = { tmp_l0, tmp_h0 };
            vst2q_u16( dst_line + x, res );
        }
        
        transform_fcc12_packed_to_fcc16_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc12_packed_mipi_to_dst_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim.cx % 2 == 0 );

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint16_t* dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;
        for( ; x < (src.dim.cx - 24); x += 16 )
        {
            uint8x8x3_t vals = vld3_u8( src_line + (x / 2) * 3 );                               // vals[0] u8-bits = u8{p0[4:B]        }, u8{p2[4:B]        }, u8{p4[4:B]} ...
                                                                                                // vals[1] u8-bits = u8{p1[4:B]        }, u8{p3[4:B]        }, u8{p5[4:B]},...
                                                                                                // vals[2] u8-bits = u8{p0[0:3],p1[0:3]}, u8{p2[0:3],p3[0:3]}, 
            uint16x8_t low = vmovl_u8( vals.val[0] );                                           // u8-bits = u16{ p0[4:B], 0 }, u16{ p2[4:B], 0 }, u16{ p4[4:B], 0 }
            uint16x8_t hig = vmovl_u8( vals.val[1] );                                           // u8-bits = u16{ p1[4:B], 0 }, u16{ p3[4:B], 0 }, u16{ p5[4:B], 0 }
            uint16x8_t mid = vmovl_u8( vals.val[2] );                                           // u8-bits = u8{p0[0:3],p1[0:3], 0, 0 }, u8{p2[0:3],p3[0:4], 0, 0 },  ....

            uint16x8_t mid_hi = vandq_u16( mid, vdupq_n_u16( 0x00F0 ) );                        // u8-bits = u8{ 0, p1[0:3], 0, 0 }, u8{ 0, p3[0:4], 0, 0 },  ....
            uint16x8_t mid_lo = vshlq_n_u16( mid, 4 );                                          // u8-bits = u8{ 0, p0[0:3], 0, 0 }, u8{ 0, p2[0:3], 0, 0 },  ....

            auto tmp_h0 = vsliq_n_u16( mid_hi, hig, 8 );
            auto tmp_l0 = vsliq_n_u16( mid_lo, low, 8 );

            uint16x8x2_t res = { tmp_l0, tmp_h0 };
            vst2q_u16( dst_line + x, res );
        }

        transform_fcc12_mipi_to_fcc16_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

void transform_fcc12_spacked_to_dst_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert( dst.dim.cx % 2 == 0 );

    // u8 11111111'11110000'00000000

    //val |= ((uint32_t( src_ptr[0] & 0xFF ) << 4) | (uint32_t( src_ptr[1] & 0x0F ) << 12)) << 0;
    //val |= ((uint32_t( src_ptr[1] & 0xF0 ) << 0) | (uint32_t( src_ptr[2] & 0xFF ) << 8)) << 16;

    for( int y = 0; y < src.dim.cy; ++y )
    {
        uint8_t* src_line = img::get_line_start<uint8_t>( src, y );
        uint16_t* dst_line = img::get_line_start<uint16_t>( dst, y );

        int x = 0;

        for( ; x < (src.dim.cx - 24); x += 16 )
        {
            uint8x8x3_t vals = vld3_u8( src_line + (x / 2) * 3 );   // vals[0] = u8{p0[0:7]        }, u8{p2[0:7]        }, u8{p4[0:7]        } ...
                                                                    // vals[1] = u8{p0[8:B],p1[0,3]}, u8{p2[8:B],p3[0,3]}, u8{p4[8:B],p5[0,3]} ...
                                                                    // vals[2] = u8{p1[4:B]        }, u8{p3[4:B]        }, u8{p5[4:B]        } ...

            uint16x8_t low = vmovl_u8( vals.val[0] );               // u8-bits = u16{ p0[0:7], 0            }, u16{ p2[0:7], 0             }, u16{ p4[0:7], 0 }
            uint16x8_t hig = vmovl_u8( vals.val[2] );               // u8-bits = u16{ p1[4:B], 0            }, u16{ p3[4:B], 0             }, u16{ p5[4:B], 0 }
            uint16x8_t mid = vmovl_u8( vals.val[1] );               // u8-bits = u16{ p0[8:B],p1[0:3], 0, 0 }, u16{ p2[8:B], p3[0:3], 0, 0 },  ....

            uint16x8_t mid_hi = vandq_u16( mid, vdupq_n_u16( 0x00F0 ) );                        // u8-bits = u16{ 0, p1[0:3], 0, 0 }, u16{ 0, p3[0:4], 0, 0 },  ....
            uint16x8_t tmp_lo = vshlq_n_u16( low, 4 );                                          // u8-bits = u16{ 0, p0[0:7], 0    }, u16{ 0, p2[0:7], 0    }

            auto tmp_h0 = vsliq_n_u16( mid_hi, hig, 8 );
            auto tmp_l0 = vsliq_n_u16( tmp_lo, mid, 12 );

            uint16x8x2_t res = { tmp_l0, tmp_h0 };
            vst2q_u16( dst_line + x, res );
        }

        transform_fcc12_spacked_to_fcc16_c_line( src_line + (x / 2) * 3, dst_line + x, src.dim.cx - x );
    }
}

const auto shiftConstant = vreinterpret_s8_s32(vdup_n_s32(0x00020406));
const auto bitMask = vdup_n_u8(0xC0); // bit mask for upper two bits of a byte
const uint8x8_t shuffleMaskUpperBytesInit = { 0, 1, 2, 3, 5, 6, 7, 8 };
const uint8x8_t shuffleMaskLowerBytesInit = { 4, 4, 4, 4, 9, 9, 9, 9 };
const auto shuffleMaskSummand = vdup_n_u8(10);
const uint8x8_t shuffleMaskUpperBytesLastBlock = { 0xff, 0xff, 0, 1, 3, 4, 5, 6 };
const uint8x8_t shuffleMaskLowerBytesLastBlock = { 2, 2, 2, 2, 7, 7, 7, 7 };

FORCEINLINE void transform_fcc10_mipi_to_fcc16_arm_neon_subblock(uint8x8x4_t pixelsFourLanes, uint8x8_t shuffleMaskUpperBytes, uint8x8_t shuffleMaskLowerBytes, uint16_t* dstPtr)
{
    auto upperBytes = vtbl4_u8(pixelsFourLanes, shuffleMaskUpperBytes);
    auto lowerBytes = vtbl4_u8(pixelsFourLanes, shuffleMaskLowerBytes);
    auto lowerBytesShifted = vshl_u8(lowerBytes, shiftConstant);
    auto lowerBytesMasked = vand_u8(lowerBytesShifted, bitMask);
    vst2_u8(reinterpret_cast<uint8_t*>(dstPtr), { lowerBytesMasked, upperBytes }); // vst2_u8 interleaves the results
}

FORCEINLINE void transform_fcc10_mipi_to_fcc16_arm_neon_block(const uint8_t* srcPtr, uint16_t* dstPtr)
{
    uint8x8x4_t pixelsFourLanes = { vld1_u8(srcPtr + 0), vld1_u8(srcPtr + 8), vld1_u8(srcPtr + 16), vld1_u8(srcPtr + 24) };
    auto shuffleMaskUpperBytes = shuffleMaskUpperBytesInit;
    auto shuffleMaskLowerBytes = shuffleMaskLowerBytesInit;
    transform_fcc10_mipi_to_fcc16_arm_neon_subblock(pixelsFourLanes, shuffleMaskUpperBytes, shuffleMaskLowerBytes, dstPtr + 0);

    shuffleMaskUpperBytes = vadd_u8(shuffleMaskUpperBytes, shuffleMaskSummand);
    shuffleMaskLowerBytes = vadd_u8(shuffleMaskLowerBytes, shuffleMaskSummand);
    transform_fcc10_mipi_to_fcc16_arm_neon_subblock(pixelsFourLanes, shuffleMaskUpperBytes, shuffleMaskLowerBytes, dstPtr + 8);

    shuffleMaskUpperBytes = vadd_u8(shuffleMaskUpperBytes, shuffleMaskSummand);
    shuffleMaskLowerBytes = vadd_u8(shuffleMaskLowerBytes, shuffleMaskSummand);
    transform_fcc10_mipi_to_fcc16_arm_neon_subblock(pixelsFourLanes, shuffleMaskUpperBytes, shuffleMaskLowerBytes, dstPtr + 16);

    shuffleMaskUpperBytes = vadd_u8(shuffleMaskUpperBytes, shuffleMaskSummand);
    auto upperBytesBeforeLastBlock = vtbl4_u8(pixelsFourLanes, shuffleMaskUpperBytes); // the upper 6 bytes are zeroed
    auto lastBlockOfSourcePixels = vld1_u8(srcPtr + 32);
    auto upperBytesFromLastBlock = vtbl1_u8(lastBlockOfSourcePixels, shuffleMaskUpperBytesLastBlock);
    auto upperBytes = vorr_u8(upperBytesBeforeLastBlock, upperBytesFromLastBlock);
    auto lowerBytes = vtbl1_u8(lastBlockOfSourcePixels, shuffleMaskLowerBytesLastBlock);
    auto lowerBytesShifted = vshl_u8(lowerBytes, shiftConstant);
    auto lowerBytesMasked = vand_u8(lowerBytesShifted, bitMask);
    vst2_u8(reinterpret_cast<uint8_t*>(dstPtr + 24), { lowerBytesMasked, upperBytes }); // vst2_u8 interleaves the results
}


void transform_fcc10_packed_to_dst_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    assert(src.dim.cx % 4 == 0);
    const auto srcLineLengthInBytes = src.dim.cx * 10 / 8;
    constexpr auto inputBlockSize = 40;
    constexpr auto outputBlockSize = 32;
    assert(abs(src.pitch()) >= srcLineLengthInBytes);
    assert(abs(dst.pitch()) >= dst.dim.cx * sizeof(uint16_t));

    for (int y = 0; y < src.dim.cy; ++y)
    {
        auto srcPtr = img::get_line_start<const uint8_t>(src, y);
        auto dstPtr = img::get_line_start<uint16_t>(dst, y);
        auto srcLineBeginLastBlock = srcPtr + srcLineLengthInBytes - inputBlockSize;
        auto dstLineBeginLastBlock = dstPtr + dst.dim.cx - outputBlockSize;
        while (dstPtr < dstLineBeginLastBlock)
        {
            transform_fcc10_mipi_to_fcc16_arm_neon_block(srcPtr, dstPtr);

            srcPtr += inputBlockSize;
            dstPtr += outputBlockSize;
        }
        transform_fcc10_mipi_to_fcc16_arm_neon_block(srcLineBeginLastBlock, dstLineBeginLastBlock);
    }
}

void    transform_fcc12_to_fcc16_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        const auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<uint16_t>( dst, y );
        int x = 0;
        for( ; x < (src.dim.cx - 7); x += 8 )
        {
            const uint16x8_t vals = vld1q_u16( src_line + x );
            const uint16x8_t tmp0 = vshlq_n_u16( vals, 4 );

            vst1q_u16( dst_line + x, tmp0 );
        }
        for( ; x < src.dim.cx; ++x )
        {
            dst_line[x] = calc_fcc12_to_fcc16( src_line, x );
        }
    }
}

void    transform_fcc10_to_fcc16_neon_v0( img::img_descriptor dst, img::img_descriptor src )
{
    for( int y = 0; y < src.dim.cy; ++y )
    {
        const auto* src_line = img::get_line_start<const uint16_t>( src, y );
        auto* dst_line = img::get_line_start<uint16_t>( dst, y );
        int x = 0;
        for( ; x < (src.dim.cx - 7); x += 8 )
        {
            const uint16x8_t vals = vld1q_u16( src_line + x );
            const uint16x8_t tmp0 = vshlq_n_u16( vals, 6 );

            vst1q_u16( dst_line + x, tmp0 );
        }
        for( ; x < src.dim.cx; ++x )
        {
            dst_line[x] = calc_fcc10_to_fcc16( src_line, x );
        }
    }
}
}


auto img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_neon_v0( const img::img_type& dst, const img::img_type& src ) -> transform_function_type
{
    if( src.dim != dst.dim ) {
        return nullptr;
    }

    using namespace img::fcc1x_packed;

    if( !is_accepted_dst_fcc16( dst.fourcc_type() ) ) {
        return nullptr;
    }

    switch( get_fcc1x_pack_type( src.fourcc_type() ) )
    {
    case fccXX_pack_type::fcc12:            return ::transform_fcc12_to_fcc16_neon_v0;
    case fccXX_pack_type::fcc12_packed:     return ::transform_fcc12_packed_to_dst_neon_v0;
    case fccXX_pack_type::fcc12_mipi:       return ::transform_fcc12_packed_mipi_to_dst_neon_v0;
    case fccXX_pack_type::fcc12_spacked:    return ::transform_fcc12_spacked_to_dst_neon_v0;

    case fccXX_pack_type::fcc10:            return ::transform_fcc10_to_fcc16_neon_v0;
    case fccXX_pack_type::fcc10_spacked:    return nullptr; // #TODO implement transform_fcc10_spacked_to_dst_neon
    case fccXX_pack_type::fcc10_mipi:    
    {
        if (src.dim.cx < 32)
        {
            return nullptr;
        }
        else
        {
            return ::transform_fcc10_packed_to_dst_neon_v0;
        }
    }
    case fccXX_pack_type::invalid:          return nullptr;
    };
    return nullptr;
}