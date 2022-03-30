
#include "transform_impl.h"

#include "../../../libs/dutils_image/src/dutils_img_base/memcpy_image.h"
#include "../../../libs/dutils_image/src/dutils_img_filter/by_edge/by_edge.h"
#include "../../../libs/dutils_image/src/dutils_img_filter/filter/whitebalance/wb_apply.h"
#include "../../../libs/dutils_image/src/dutils_img_filter/transform/fcc1x_packed/fcc1x_packed_to_fcc.h"
#include "../../../libs/dutils_image/src/dutils_img_filter/transform/fcc1x_packed/transform_fcc1x_to_fcc8.h"
#include "../../../libs/dutils_image/src/dutils_img_filter/transform/fcc8_fcc16/transform_fcc8_fcc16.h"
#include "../../../libs/dutils_image/src/dutils_img_filter/transform/mono_to_bgr/transform_mono_to_bgr.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <vector>

namespace
{
using fcc_array = std::array<img::fourcc, 16>;


struct fcc_array2
{
    template<typename... fccs>
    constexpr fcc_array2(fccs... fcc_list) : data_ { fcc_list... }, count { sizeof...(fcc_list) }
    {
    }

    fcc_array data_;
    int count = 0;

    constexpr auto begin() const noexcept
    {
        return data_.begin();
    }
    constexpr auto end() const noexcept
    {
        return data_.begin() + count;
    }

    bool has_fcc(img::fourcc fcc) const noexcept
    {
        return std::any_of(begin(), end(), [fcc](auto v) { return v == fcc; });
    }
};

struct transform_path
{
    fcc_array2 src_fcc;
    fcc_array2 dst_fcc;
};

using namespace img;


// clang-format off
static const constexpr transform_path transform_entries[] =
{
    {
        { fourcc::MONO8 },
        { fourcc::MONO8, fourcc::BGRA32 },
    },
    {
        {
            fourcc::MONO10,
            fourcc::MONO10_MIPI_PACKED,
            fourcc::MONO10_SPACKED,
            fourcc::MONO12,
            fourcc::MONO12_MIPI_PACKED,
            fourcc::MONO12_SPACKED,
            fourcc::MONO12_PACKED,
            fourcc::MONO16,
        },
        { fourcc::MONO8, fourcc::MONO16, fourcc::BGRA32 }
    },
    {
        { fourcc::BGGR8, },
        { fourcc::BGGR8, fourcc::BGRA32 }
    },
    {
        {
            fourcc::BGGR10,
            fourcc::BGGR10_SPACKED,
            fourcc::BGGR10_MIPI_PACKED,
            fourcc::BGGR12,
            fourcc::BGGR12_PACKED,
            fourcc::BGGR12_SPACKED,
            fourcc::BGGR12_MIPI_PACKED,
            fourcc::BGGR16,
        },
        { fourcc::BGGR8, fourcc::BGGR16, fourcc::BGRA32 }
    },
    {
        { fourcc::GBRG8, },
        { fourcc::GBRG8, fourcc::BGRA32 }
    },
    {
        {
            fourcc::GBRG10,
            fourcc::GBRG10_SPACKED,
            fourcc::GBRG10_MIPI_PACKED,
            fourcc::GBRG12,
            fourcc::GBRG12_PACKED,
            fourcc::GBRG12_SPACKED,
            fourcc::GBRG12_MIPI_PACKED,
            fourcc::GBRG16,
        },
        { fourcc::GBRG8, fourcc::GBRG16, fourcc::BGRA32 }
    },
    {
        { fourcc::RGGB8, },
        { fourcc::RGGB8, fourcc::BGRA32 }
    },
    {
        {
            fourcc::RGGB10,
            fourcc::RGGB10_SPACKED,
            fourcc::RGGB10_MIPI_PACKED,
            fourcc::RGGB12,
            fourcc::RGGB12_PACKED,
            fourcc::RGGB12_SPACKED,
            fourcc::RGGB12_MIPI_PACKED,
            fourcc::RGGB16,
        },
        { fourcc::RGGB8, fourcc::RGGB16, fourcc::BGRA32 }
    },
    {
        { fourcc::GRBG8, },
        { fourcc::GRBG8, fourcc::BGRA32 }
    },
    {
        {
            fourcc::GRBG10,
            fourcc::GRBG10_SPACKED,
            fourcc::GRBG10_MIPI_PACKED,
            fourcc::GRBG12,
            fourcc::GRBG12_PACKED,
            fourcc::GRBG12_SPACKED,
            fourcc::GRBG12_MIPI_PACKED,
            fourcc::GRBG16,
        },
        { fourcc::GRBG8, fourcc::GRBG16, fourcc::BGRA32 }
    },
};
// clang-format on

void append_unique(std::vector<img::fourcc>& vec, const fcc_array2& arr)
{
    for (auto fcc : arr)
    {
        const bool already_present = std::any_of(
            vec.begin(), vec.end(), [fcc](auto fcc_in_vec) { return fcc == fcc_in_vec; });
        if (!already_present)
            vec.push_back(fcc);
    }
}

} // namespace

auto tcamconvert::tcamconvert_get_all_input_fccs() -> std::vector<img::fourcc>
{
    std::vector<img::fourcc> rval;
    for (auto e : transform_entries) { append_unique(rval, e.src_fcc); }
    return rval;
}


auto tcamconvert::tcamconvert_get_all_output_fccs() -> std::vector<img::fourcc>
{
    std::vector<img::fourcc> rval;
    for (auto e : transform_entries) { append_unique(rval, e.dst_fcc); }
    return rval;
}

auto tcamconvert::tcamconvert_get_supported_input_fccs(img::fourcc dst_fcc)
    -> std::vector<img::fourcc>
{
    std::vector<img::fourcc> rval;
    for (auto e : transform_entries)
    {
        if (e.dst_fcc.has_fcc(dst_fcc))
        {
            append_unique(rval, e.src_fcc);
        }
    }
    return rval;
}

auto tcamconvert::tcamconvert_get_supported_output_fccs(img::fourcc src_fcc)
    -> std::vector<img::fourcc>
{
    std::vector<img::fourcc> rval;
    for (auto e : transform_entries)
    {
        if (e.src_fcc.has_fcc(src_fcc))
        {
            append_unique(rval, e.dst_fcc);
        }
    }
    return rval;
}


static auto find_transform_unary_wb_func(img::img_type type)
{
#if defined DUTILS_ARCH_ARM
    auto wb_func = img_filter::whitebalance::get_apply_img_neon(type);
#elif defined DUTILS_ARCH_SSE41
    auto wb_func = img_filter::whitebalance::get_apply_img_sse41(type);
#elif
    auto wb_func = img_filter::whitebalance::get_apply_img_c(type);
#endif
    return wb_func;
}


static auto find_transform_mono_to_bgr_func(img::img_type dst_type, img::img_type src_type)
{
#if defined DUTILS_ARCH_ARM
    auto func = img_filter::transform::get_transform_mono_to_bgr_neon(dst_type, src_type);
#elif defined DUTILS_ARCH_SSE41
    auto func = img_filter::transform::get_transform_mono_to_bgr_sse41(dst_type, src_type);
#elif
    auto func = img_filter::transform::get_transform_mono_to_bgr_c(dst_type, src_type);
#endif
    return func;
}

static auto find_transform_function_type(img::img_type dst_type, img::img_type src_type)
    -> img_filter::transform_function_type
{
    using func_type =
        img_filter::transform_function_type (*)(const img::img_type&, const img::img_type&);

    static func_type func_list[] = {
#if defined DUTILS_ARCH_ARM
        img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_neon_v0,
        img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_neon_v0,
        img_filter::transform::get_transform_fcc8_to_fcc16_neon,
        img_filter::transform::get_transform_fcc16_to_fcc8_neon,
#else
        img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_ssse3,
        img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_ssse3,
        img_filter::transform::get_transform_fcc8_to_fcc16_sse41,
        img_filter::transform::get_transform_fcc16_to_fcc8_sse41,
#endif
        img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc8_c,
        img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_c,
        img_filter::transform::get_transform_fcc8_to_fcc16_c,
        img_filter::transform::get_transform_fcc16_to_fcc8_c,
    };

    for (auto func : func_list)
    {
        if (auto res = func(dst_type, src_type); res)
        {
            return res;
        }
    }
    return nullptr;
}

static auto find_transform_function_wb_type(img::img_type dst_type, img::img_type src_type)
    -> std::function<void(const img::img_descriptor& dst,
                          const img::img_descriptor& src,
                          img_filter::filter_params& params)>
{
    using func_type =
        img_filter::transform_function_param_type (*)(const img::img_type&, const img::img_type&);

    static func_type func_list[] = {
#if defined DUTILS_ARCH_ARM
        img_filter::transform::fcc1x_packed::get_transform_fcc1x_to_fcc8_neon_v0,
    //img_filter::transform::fcc1x_packed::get_transform_fcc10or12_packed_to_fcc16_neon_v0,
    //img_filter::transform::get_transform_fcc8_to_fcc16_neon,
    //img_filter::transform::get_transform_fcc16_to_fcc8_wb_neon,
#else
#endif
        img_filter::transform::fcc1x_packed::get_transform_fcc1x_to_fcc8_c,
    };

    for (auto func : func_list)
    {
        if (auto res = func(dst_type, src_type); res)
        {
            return res;
        }
    }

    auto transform_only_func = find_transform_function_type(dst_type, src_type);
    auto wb_func = find_transform_unary_wb_func(dst_type);

    auto transform_func = [transform_only_func, wb_func](const img::img_descriptor& dst,
                                                         const img::img_descriptor& src,
                                                         img_filter::filter_params& params)
    {
        transform_only_func(dst, src);
        wb_func(dst, params.whitebalance);
    };
    return transform_func;
}

static auto find_bayer8_to_bgra_func(const img::img_type& dst_type, const img::img_type& src_type)
    -> tcamconvert::transform_binary_func
{
#if defined DUTILS_ARCH_ARM
    auto func = img_filter::transform::by_edge::get_transform_by8_to_dst_neon(dst_type, src_type);
#elif defined DUTILS_ARCH_SSE41
    auto func = img_filter::transform::by_edge::get_transform_by8_to_dst_sse41(dst_type, src_type);
#elif
    auto func = img_filter::transform::by_edge::get_transform_by8_to_dst_c(dst_type, src_type);
#endif
    return [func](const img::img_descriptor& dst, const img::img_descriptor& src)
    {
        static const img_filter::transform::by_edge::options opt = { {}, false, false };

        func(dst, src, opt);
    };
}

enum class transform_context_mode
{
    unary_mono,
    unary_bayer,
    binary_mono,
    binary_bayer,
    binary_rgb,
};

static auto get_transform_context_mode(img::img_type src_type, img::img_type dst_type)
    -> transform_context_mode
{
    enum class color_mode
    {
        bayer,
        mono
    };
    auto clr_mode = img::is_mono_fcc(src_type.fourcc_type()) ? color_mode::mono : color_mode::bayer;

    if (src_type.fourcc_type() == dst_type.fourcc_type())
    {
        if (clr_mode == color_mode::mono)
        {
            return transform_context_mode::unary_mono;
        }
        return transform_context_mode::unary_bayer;
    }

    if (dst_type.fourcc_type() == img::fourcc::BGRA32)
    {
        return transform_context_mode::binary_rgb;
    }

    if (clr_mode == color_mode::mono)
    {
        return transform_context_mode::binary_mono;
    }
    return transform_context_mode::binary_bayer;
}

bool tcamconvert::transform_context::setup(img::img_type src_type, img::img_type dst_type)
{
    transform_unary_wb_func_ = nullptr;
    transfrom_binary_mono_func_ = nullptr;
    transform_fccXX_to_dst_func_ = nullptr;

    transform_intermediate_buffer_ = {};

    switch (get_transform_context_mode(src_type, dst_type))
    {
        case transform_context_mode::unary_mono:
            break;
        case transform_context_mode::unary_bayer:
        {
            transform_unary_wb_func_ = find_transform_unary_wb_func(dst_type);
            assert(transform_unary_wb_func_ != nullptr);

            return transform_unary_wb_func_ != nullptr;
        }
        case transform_context_mode::binary_mono:
        {
            transfrom_binary_mono_func_ = find_transform_function_type(dst_type, src_type);
            assert(transfrom_binary_mono_func_ != nullptr);

            return transfrom_binary_mono_func_ != nullptr;
        }
        case transform_context_mode::binary_bayer:
        {
            transform_fccXX_to_dst_func_ = find_transform_function_wb_type(dst_type, src_type);
            assert(transform_fccXX_to_dst_func_ != nullptr);

            return transform_fccXX_to_dst_func_ != nullptr;
        }
        case transform_context_mode::binary_rgb:
        {
            if (src_type.fourcc_type() == fourcc::MONO8) // MONO8 to BGRA32
            {
                auto transform_to_bgra_func = find_transform_mono_to_bgr_func(dst_type, src_type);
                assert(transform_to_bgra_func != nullptr);

                transform_fccXX_to_dst_func_ =
                    [transform_to_bgra_func](const img::img_descriptor& dst,
                                             const img::img_descriptor& src,
                                             img_filter::filter_params& /*params*/)
                {
                    assert(src.fourcc_type() == img::fourcc::MONO8);
                    assert(dst.fourcc_type() == img::fourcc::BGRA32);

                    transform_to_bgra_func(dst, src);
                };
                return transform_fccXX_to_dst_func_ != nullptr;
            }
            else if (
                img::is_mono_fcc(
                    src_type.fourcc_type())) // MONOXX to BGRA32, done via MONOXX -> MONO8 -> BGRA32
            {
                auto transform_intermediate_type =
                    img::make_img_type(img::fourcc::MONO8, src_type.dim);

                transform_intermediate_buffer_.resize(transform_intermediate_type.buffer_length);

                auto transfrom_to_mono8 =
                    find_transform_function_type(transform_intermediate_type, src_type);
                assert(transfrom_to_mono8 != nullptr);

                auto transform_to_bgra_func =
                    find_transform_mono_to_bgr_func(dst_type, transform_intermediate_type);
                assert(transform_to_bgra_func != nullptr);

                transform_fccXX_to_dst_func_ =
                    [transform_intermediate_type, transfrom_to_mono8, transform_to_bgra_func, this](
                        const img::img_descriptor& dst,
                        const img::img_descriptor& src,
                        img_filter::filter_params& /*params*/)
                {
                    assert(dst.fourcc_type() == img::fourcc::BGRA32);

                    auto mono8_img_desc = img::make_img_desc_from_linear_memory(
                        transform_intermediate_type, transform_intermediate_buffer_.data());

                    transfrom_to_mono8(mono8_img_desc, src);

                    transform_to_bgra_func(dst, mono8_img_desc);
                };
                return transform_fccXX_to_dst_func_ != nullptr;
            }
            else if (img::is_by8_fcc(src_type.fourcc_type())) // Bayer8 -> BGRA32
            {
                auto wb_func =
                    find_transform_unary_wb_func(src_type); // whitebalance on src image func
                assert(wb_func != nullptr);

                auto transform_by8_to_bgra_func = find_bayer8_to_bgra_func(dst_type, src_type);
                assert(transform_by8_to_bgra_func != nullptr);

                transform_fccXX_to_dst_func_ =
                    [transform_by8_to_bgra_func, wb_func](const img::img_descriptor& dst,
                                                          const img::img_descriptor& src,
                                                          img_filter::filter_params& params)
                {
                    assert(dst.fourcc_type() == img::fourcc::BGRA32);

                    wb_func(src, params.whitebalance);
                    transform_by8_to_bgra_func(dst, src);
                };
                return transform_fccXX_to_dst_func_ != nullptr;
            }
            else if (!img::is_by8_fcc(src_type.fourcc_type())) // bayerXX -> BGRA32, done via bayerXX -> bayer8 -> BGRA32
            {
                auto transform_intermediate_type = img::make_img_type(
                    img::by_transform::convert_bayer_fcc_to_bayer8_fcc(src_type.fourcc_type()),
                    src_type.dim);

                transform_intermediate_buffer_.resize(transform_intermediate_type.buffer_length);

                auto transform_byXX_to_byYY_func =
                    find_transform_function_wb_type(transform_intermediate_type, src_type);
                assert(transform_byXX_to_byYY_func != nullptr);
                auto transform_by8_to_bgra_func =
                    find_bayer8_to_bgra_func(dst_type, transform_intermediate_type);
                assert(transform_by8_to_bgra_func != nullptr);


                transform_fccXX_to_dst_func_ = [transform_by8_to_bgra_func,
                                                transform_byXX_to_byYY_func,
                                                transform_intermediate_type,
                                                this](const img::img_descriptor& dst,
                                                      const img::img_descriptor& src,
                                                      img_filter::filter_params& params)
                {
                    assert(dst.fourcc_type() == img::fourcc::BGRA32);

                    auto by8_img_desc = img::make_img_desc_from_linear_memory(
                        transform_intermediate_type, transform_intermediate_buffer_.data());

                    transform_byXX_to_byYY_func(by8_img_desc, src, params);

                    transform_by8_to_bgra_func(dst, by8_img_desc);
                };

                return transform_fccXX_to_dst_func_ != nullptr;
            }
        }
    }
    return true;
}

void tcamconvert::transform_context::transform(const img::img_descriptor& src,
                                               const img::img_descriptor& dst,
                                               const img_filter::whitebalance_params& params)
{
    if (transform_fccXX_to_dst_func_ == nullptr && transfrom_binary_mono_func_ == nullptr)
    {
        img::memcpy_image(dst, src);
        if (transform_unary_wb_func_ && params.apply)
        {
            transform_unary_wb_func_(dst, params);
        }
    }
    else
    {
        auto dst_ = dst;
        if (dst.fourcc_type() == img::fourcc::BGRA32)
        {
            dst_ = img::flip_image_in_img_desc(dst);
        }

        if (transform_fccXX_to_dst_func_)
        {
            img_filter::filter_params tmp = { params };

            transform_fccXX_to_dst_func_(dst_, src, tmp);
        }
        else
        {

            assert(transfrom_binary_mono_func_ != nullptr);

            transfrom_binary_mono_func_(dst_, src);
        }
    }
}

void tcamconvert::transform_context::filter(const img::img_descriptor& src,
                                            const img_filter::whitebalance_params& params)
{
    if (transform_unary_wb_func_ && params.apply)
    {
        transform_unary_wb_func_(src, params);
    }
}
