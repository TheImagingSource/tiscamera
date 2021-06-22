

#pragma once

#include <cstdint>

#include "fcc1x_packed_to_fcc16_internal.h"

namespace img::fcc1x_packed
{
    using fccXX_to_pixel_function = uint16_t( * )(const void*, int);

    inline auto    get_fccXX_to_fcc16_func( img::fourcc fcc ) noexcept -> fccXX_to_pixel_function
    {
        auto pack_info = img::fcc1x_packed::get_fcc1x_pack_info( fcc );

        using namespace fcc1x_packed_internal;

        assert( pack_info.pack_type != fccXX_pack_type::invalid );

        switch( pack_info.pack_type )
        {
        case fccXX_pack_type::fcc10:            return calc_fcc10_to_fcc16;
        case fccXX_pack_type::fcc10_spacked:    return calc_fcc10_spacked_to_fcc16;
        case fccXX_pack_type::fcc10_mipi:       return calc_fcc10_packed_mipi_to_fcc16;

        case fccXX_pack_type::fcc12:            return calc_fcc12_to_fcc16;
        case fccXX_pack_type::fcc12_packed:     return calc_fcc12_packed_to_fcc16;
        case fccXX_pack_type::fcc12_mipi:       return calc_fcc12_mipi_to_fcc16;
        case fccXX_pack_type::fcc12_spacked:    return calc_fcc12_spacked_to_fcc16;

        case fccXX_pack_type::invalid:          return nullptr;
        };
        return nullptr;
    }
}
