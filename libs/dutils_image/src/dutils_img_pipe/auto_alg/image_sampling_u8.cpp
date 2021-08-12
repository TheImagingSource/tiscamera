
#include "image_sampling_u8.h"

#include <dutils_img/image_bayer_pattern.h>
#include "../../dutils_img_base/interop_private.h"
#include "../../dutils_img_filter/transform/fcc1x_packed/fcc1x_packed_to_fcc8_internal.h"
#include "../../dutils_img_filter/transform/pwl/transform_pwl_functions.h"

using namespace auto_alg::impl;

using by_pattern = img::by_transform::by_pattern;

using img::by_transform::by_pattern_alg::next_pixel;
using img::by_transform::by_pattern_alg::next_line;

struct by_sample_entries 
{
    uint8_t v00, v01, v10, v11;
};

static constexpr auto to_auto_sample_entry( by_pattern pat, by_sample_entries e ) noexcept -> auto_sample_entry
{
    switch( pat )
    {     //											r      gr     b      gb
    case by_pattern::RG:    return auto_sample_entry{ e.v00, e.v01, e.v11, e.v10 };
    case by_pattern::GR:    return auto_sample_entry{ e.v01, e.v00, e.v10, e.v11 };
    case by_pattern::BG:    return auto_sample_entry{ e.v11, e.v10, e.v00, e.v01 };
    case by_pattern::GB:    return auto_sample_entry{ e.v10, e.v11, e.v01, e.v00 };
    }
    UNREACHABLE_CODE();
}

using bayer_pix_sample_func_type = by_sample_entries (*)(int x, const uint8_t* cur_line, const uint8_t* nxt_line );

static void    auto_sample_bayer_image( const img::img_descriptor& image, auto_sample_points& points, bayer_pix_sample_func_type to_auto_sample_entry_func )
{
    const auto step_dim = auto_alg::impl::calc_image_sample_step_dim( image );
    if( step_dim.empty() ) {
        return;
    }

    const auto img_pat = img::by_transform::convert_bayer_fcc_to_pattern( image.fourcc_type() );

    int cnt = 0;	// max is 42 * 32
    for( int y = step_dim.cy; y < (image.dim.cy - 1); y += step_dim.cy )
    {
        const auto line_pat = (y & 0x1) ? next_line( img_pat ) : img_pat;

        const uint8_t* cur_line = img::get_line_start<uint8_t>( image, y + 0 );
        const uint8_t* nxt_line = img::get_line_start<uint8_t>( image, y + 1 );

        for( int x = step_dim.cx; x < (image.dim.cx - 1); x += step_dim.cx )
        {
            const auto pix_pat = (x & 0x1) ? next_pixel( line_pat ) : line_pat;

            const auto bayer_sample_data = to_auto_sample_entry_func( x, cur_line, nxt_line );

            points.samples[cnt] = to_auto_sample_entry( pix_pat, bayer_sample_data );
            ++cnt;
        }
    }
    points.cnt = cnt;
}

static void	auto_sample_by8img( const img::img_descriptor& image, auto_sample_points& points )
{
    const auto to_entry_by8 = []( int x, const uint8_t* cur_line, const uint8_t* nxt_line ) noexcept -> by_sample_entries
    {
        return { cur_line[x + 0], cur_line[x + 1], nxt_line[x + 0], nxt_line[x + 1] };
    };
    auto_sample_bayer_image( image, points, to_entry_by8 );
}

static void	auto_sample_by16img( const img::img_descriptor& image, auto_sample_points& points )
{
    const auto to_entry_by16 = []( int x, const uint8_t* cur_line, const uint8_t* nxt_line ) noexcept -> by_sample_entries
    {
        uint8_t v00 = reinterpret_cast<const uint16_t*>( cur_line )[x + 0] >> 8;    // this should read the high byte for a by16 entry
        uint8_t v01 = reinterpret_cast<const uint16_t*>( cur_line )[x + 1] >> 8;
        uint8_t v10 = reinterpret_cast<const uint16_t*>( nxt_line )[x + 0] >> 8;
        uint8_t v11 = reinterpret_cast<const uint16_t*>( nxt_line )[x + 1] >> 8;
        return { v00, v01, v10, v11 };
    };
    auto_sample_bayer_image( image, points, to_entry_by16 );
}

template<class TPack>
void auto_sample_img_packed( const img::img_descriptor& image, auto_alg::impl::auto_sample_points& points )
{
    const auto step_dim = auto_alg::impl::calc_image_sample_step_dim( image );
    if( step_dim.empty() ) {
        return;
    }

    int cnt = 0;	// max is 42 * 32
    for( int y = step_dim.cy; y < image.dim.cy; y += step_dim.cy )
    {
        const auto* cur_line = img::get_line_start<TPack>( image, y );

        for( int x = step_dim.cx; x < (image.dim.cx - 1); x += step_dim.cx )
        {
            TPack pix = cur_line[x];

            points.samples[cnt] = auto_sample_entry{ (uint8_t)pix.r, (uint8_t)pix.g, (uint8_t)pix.b, (uint8_t)pix.g };
            ++cnt;
        }
    }
    points.cnt = cnt;
}

using to_sample_entries_func = uint8_t (*)(const void*, int);

template<to_sample_entries_func func>
by_sample_entries by10or12_to_sample_entries( int x, const uint8_t* cur_line, const uint8_t* nxt_line )
{
    return by_sample_entries{
        func( cur_line, x + 0 ),
        func( cur_line, x + 1 ),
        func( nxt_line, x + 0 ),
        func( nxt_line, x + 1 ),
    };
}

bool auto_alg::impl::can_auto_sample_by_imgu8( img::fourcc fcc )
{
    if( img::is_by8_fcc( fcc ) ) {
        return true;
    }
    if( img::is_by16_fcc( fcc ) ) {
        return true;
    }
    if( img::is_mono_fcc( fcc ) ) { // we don't have a dedicated bayer func, so use this
        return false;
    }

    auto pack_type = img::fcc1x_packed::get_fcc1x_pack_type( fcc );
    if( pack_type != img::fcc1x_packed::fccXX_pack_type::invalid ) {
        return true;
    }
    switch( fcc )
    {
    case img::fourcc::BGR24:
    case img::fourcc::BGRA32:
        return true;
    default:
        break;
    }

    return false;
}


void auto_alg::impl::auto_sample_by_imgu8( const img::img_descriptor& image, auto_sample_points& points )
{
    assert( !image.empty() );
    assert( can_auto_sample_by_imgu8( image.fourcc_type() ) );
    
    points.cnt = 0;

    if( img::is_by8_fcc( image.fourcc_type() ) ) {
        auto_sample_by8img( image, points );
    } else if( img::is_by16_fcc( image.fourcc_type() ) ) {
        auto_sample_by16img( image, points );
    } else if( image.fourcc_type() == img::fourcc::BGR24 ) {
        auto_sample_img_packed<img::pixel_type::BGR24>( image, points );
    } else if( image.fourcc_type() == img::fourcc::BGRA32 ) {
        auto_sample_img_packed<img::pixel_type::BGRA32>( image, points );
    }
    else
    {
        using namespace img::fcc1x_packed;
        using namespace fcc1x_packed_internal;
        if( auto pack_type = get_fcc1x_pack_type( image.fourcc_type() ); pack_type != fccXX_pack_type::invalid )
        {
            switch( pack_type )
            {
            case fccXX_pack_type::fcc10:			auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<calc_fcc10_to_fcc8> ); return;
            case fccXX_pack_type::fcc10_spacked:    auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<calc_fcc10_spacked_to_fcc8> ); return;
            case fccXX_pack_type::fcc10_mipi:       auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<calc_fcc10_mipi_to_fcc8> ); return;

            case fccXX_pack_type::fcc12:			auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<calc_fcc12_to_fcc8> ); return;
            case fccXX_pack_type::fcc12_packed:     auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<calc_fcc12_packed_to_fcc8> ); return;
            case fccXX_pack_type::fcc12_mipi:       auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<calc_fcc12_mipi_to_fcc8> ); return;
            case fccXX_pack_type::fcc12_spacked:    auto_sample_bayer_image( image, points, &by10or12_to_sample_entries<calc_fcc12_spacked_to_fcc8> ); return;

            case fccXX_pack_type::invalid:			assert( false ); return;
            }
        }
        else
        {
            assert( false && "Invalid image format specified" );
        }
    }
}

using mono_sample_pixel = uint8_t( * )( const void* line_start, int x );

static auto_alg::impl::resulting_brightness	        auto_sample_mono_img_( const img::img_descriptor& image, mono_sample_pixel func )
{
    const auto step_dim = auto_alg::impl::calc_image_sample_step_dim( image );
    if( step_dim.empty() ) {
        return auto_alg::impl::resulting_brightness::invalid();
    }

    int cnt = 0;

    int cnt_y_gt_240 = 0;
    int64_t pixel_accu = 0;
    for( int y = step_dim.cy; y < image.dim.cy; y += step_dim.cy )
    {
        const uint8_t* line = img::get_line_start( image, y );
        for( int x = step_dim.cx; x < image.dim.cx; x += step_dim.cx )
        {
            const uint8_t val = func( line, x );

            ++cnt;
            pixel_accu += val;
            if( val >= 240 ) {
                ++cnt_y_gt_240;
            }
        }
    }

    assert( cnt > 0 );

    float div = 1.f / cnt;

    return { pixel_accu / 255.f * div, cnt_y_gt_240 * div };
}

auto_alg::impl::resulting_brightness	auto_alg::impl::auto_sample_mono_imgu8( const img::img_descriptor& image )
{
    if( image.fourcc_type() == img::fourcc::MONO8 )
    {
        return auto_sample_mono_img_( image, []( const void* line, int x ) -> uint8_t { return reinterpret_cast<const uint8_t*>(line)[x]; } );
    }
    else if( image.fourcc_type() == img::fourcc::MONO16 )
    {
        return auto_sample_mono_img_( image, []( const void* line, int x ) -> uint8_t { return reinterpret_cast<const uint16_t*>(line)[x] >> 8; } );
    }
    else if( img::is_mono_fcc( image.fourcc_type() ) )
    {
        using namespace img::fcc1x_packed;
        using namespace fcc1x_packed_internal;

        switch( get_fcc1x_pack_type( image.fourcc_type() ) )
        {
        case fccXX_pack_type::fcc10:			    return auto_sample_mono_img_( image, &calc_fcc10_to_fcc8 );
        case fccXX_pack_type::fcc10_spacked:        return auto_sample_mono_img_( image, &calc_fcc10_spacked_to_fcc8 );
        case fccXX_pack_type::fcc10_mipi:           return auto_sample_mono_img_( image, &calc_fcc10_mipi_to_fcc8 );

        case fccXX_pack_type::fcc12:			    return auto_sample_mono_img_( image, &calc_fcc12_to_fcc8 );
        case fccXX_pack_type::fcc12_packed:         return auto_sample_mono_img_( image, &calc_fcc12_packed_to_fcc8 );
        case fccXX_pack_type::fcc12_mipi:           return auto_sample_mono_img_( image, &calc_fcc12_mipi_to_fcc8 );
        case fccXX_pack_type::fcc12_spacked:        return auto_sample_mono_img_( image, &calc_fcc12_spacked_to_fcc8 );

        case fccXX_pack_type::invalid:				assert( false ); return {};
        }
        UNREACHABLE_CODE();
    }

    return auto_alg::impl::resulting_brightness::invalid();
}
