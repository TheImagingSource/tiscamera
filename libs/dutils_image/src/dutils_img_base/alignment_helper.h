

#pragma once

#include <dutils_img/dutils_img.h>

namespace simd
{
    template<size_t alignment_value>
    inline unsigned int	calc_misalignment_bytecnt( void* p, unsigned int offset ) noexcept
    {
        return (reinterpret_cast<uintptr_t>(p) + offset) % alignment_value;
    }

    /** Adjusts the passed pointer (+offset) to the next pointer value which is a multiple of the template parameter alignment_value.
     *  Note: This function returns ptr when the pointer is already aligned.
     *
     * @param ptr               The pointer to adjust
     * @param offset            The offset to add to ptr, for convenience.
     * @param alignment_value   The alignment value to reach. e.g. 64 to get the pointer to point to the start of a cache line
     */
    template<size_t alignment_value>
    inline void*	adjust_pointer_to_nxt_align( void* ptr, std::ptrdiff_t offset = 0 ) noexcept
    {
        const auto address = reinterpret_cast<uintptr_t>(ptr);
        std::ptrdiff_t mod = ((address + offset) % alignment_value);
        if( mod == 0 ) {
            return ptr;
        }
        return static_cast<void*>( static_cast<uint8_t*>(ptr) + alignment_value - mod );
    }

    template<size_t alignment_value>
    inline bool	    is_aligned_for_stream( void* p, int pitch ) noexcept
    {
        const auto address = reinterpret_cast<uintptr_t>(p);
        if( address % alignment_value ) {
            return false;
        }
        const auto pitch_tmp = static_cast<size_t>(pitch < 0 ? -pitch : pitch);
        if( pitch_tmp % alignment_value ) {
            return false;
        }
        return true;
    }

    template<size_t alignment_value>
    inline bool	    is_aligned_for_stream( void* p ) noexcept
    {
        const auto address = reinterpret_cast<uintptr_t>(p);
        if( address % alignment_value ) {
            return false;
        }
        return true;
    }

    inline bool	is_aligned_for_sse_stream( void* p, int pitch ) noexcept
    {
        return is_aligned_for_stream<16>( p, pitch );
    }
    inline bool	is_aligned_for_avx_stream( void* p, int pitch ) noexcept
    {
        return is_aligned_for_stream<32>( p, pitch );
    }

    inline bool is_aligned_for_sse_stream( img::img_descriptor const & dsc ) noexcept {
        return is_aligned_for_sse_stream( dsc.data(), dsc.pitch() );
    }
    inline bool is_aligned_for_avx_stream( img::img_descriptor const & dsc ) noexcept {
        return is_aligned_for_avx_stream( dsc.data(), dsc.pitch() );
    }


} // simd
