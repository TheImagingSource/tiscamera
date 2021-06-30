
#include "tcamconvert_context.h"

#include "../../lib/dutils_image/src/dutils_img_filter/filter/whitebalance/wb_apply.h"
#include "../../lib/dutils_image/src/dutils_img_filter/transform/fcc1x_packed/fcc1x_packed_to_fcc.h"
#include "../../lib/dutils_image/src/dutils_img_filter/transform/fcc1x_packed/transform_fcc1x_to_fcc8.h"
#include "../../lib/dutils_image/src/dutils_img_filter/transform/fcc8_fcc16/transform_fcc8_fcc16.h"

#include <chrono>

static auto find_wb_func(img::img_type type)
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
    //img_filter::transform::fcc1x_packed::get_transform_fcc1x_to_fcc8_neon_v0,
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
    auto wb_func = find_wb_func(dst_type);

    auto transform_func = [transform_only_func, wb_func](const img::img_descriptor& dst,
                                                         const img::img_descriptor& src,
                                                         img_filter::filter_params& params) {
        transform_only_func(dst, src);
        wb_func(dst, params.whitebalance);
    };
    return transform_func;
}

namespace
{
using namespace tcamprop_system;
static const property_desc balance_white_auto = { "BalanceWhiteAuto",
                                                  prop_type::menu,
                                                  "AnalogControl",
                                                  "BalanceWhiteAuto",
                                                  { "Off", "Continuous", "Once" } };
static const property_desc balance_white_red = { "BalanceWhiteRed",
                                                 prop_type::real,
                                                 "AnalogControl",
                                                 "BalanceWhiteAuto" };
static const property_desc balance_white_green = { "BalanceWhiteGreen",
                                                   prop_type::real,
                                                   "AnalogControl",
                                                   "BalanceWhiteAuto" };
static const property_desc balance_white_blue = { "BalanceWhiteBlue",
                                                  prop_type::real,
                                                  "AnalogControl",
                                                  "BalanceWhiteAuto" };
static const constexpr prop_range_real balance_white_channel_range =
    prop_range_real { 0.0, 4.0, 1.0, 0.01 };
} // namespace

tcamconvert::tcamconvert_context_base::tcamconvert_context_base()
{
    auto flags_func = [this](bool locked) {
        if (get_color_mode() == color_mode::bayer)
        {
            auto flags = prop_flags::implemented | prop_flags::available;
            if (locked)
            {
                flags |= prop_flags::locked;
            }
            return flags;
        }
        return prop_flags::noflags;
    };

    prop_list_.register_menu(
        balance_white_auto,
        [this](int val) {
            wb_auto_ = val == 1;
            if (val == 2)
            {
                wb_once_ = true;
            }
            return std::error_code {};
        },
        [this] { return wb_auto_ ? 1 : 0; },
        [&] { return flags_func(false); },
        wb_auto_ ? 1 : 0);
    prop_list_.register_double(
        balance_white_red,
        [this](double val) {
            wb_channels_.r = val;
            return std::error_code {};
        },
        [this] { return wb_channels_.r; },
        [&] { return flags_func(wb_auto_); },
        balance_white_channel_range);
    prop_list_.register_double(
        balance_white_green,
        [this](double val) {
            wb_channels_.g = val;
            return std::error_code {};
        },
        [this] { return wb_channels_.g; },
        [&] { return flags_func(wb_auto_); },
        balance_white_channel_range);
    prop_list_.register_double(
        balance_white_blue,
        [this](double val) {
            wb_channels_.b = val;
            return std::error_code {};
        },
        [this] { return wb_channels_.b; },
        [&] { return flags_func(wb_auto_); },
        balance_white_channel_range);
}

bool tcamconvert::tcamconvert_context_base::setup(img::img_type src_type, img::img_type dst_type)
{
    auto clr_mode =
        img::is_mono_fcc(src_type_.fourcc_type()) ? color_mode::mono : color_mode::bayer;

    transform_unary_func_ = nullptr;
    transfrom_binary_mono_func_ = nullptr;
    transform_binary_color_func_ = nullptr;

    if (src_type_.fourcc_type() == dst_type_.fourcc_type())
    {
        if (clr_mode != color_mode::mono)
        {
            auto func_unary = find_wb_func(dst_type);
            if (func_unary == nullptr)
            {
                return false;
            }
            transform_unary_func_ = func_unary;
        }
    }
    else
    {
        if (clr_mode == color_mode::mono)
        {
            auto func = find_transform_function_type(dst_type, src_type);
            if (func == nullptr)
            {
                return false;
            }
            transfrom_binary_mono_func_ = func;
        }
        else
        {
            auto func = find_transform_function_wb_type(dst_type, src_type);
            if (func == nullptr)
            {
                return false;
            }
            transform_binary_color_func_ = func;
        }
    }

    this->src_type_ = src_type;
    this->dst_type_ = dst_type;

    auto_alg::reset_auto_pass_context(*auto_pass_state_);

    return true;
}

auto tcamconvert::tcamconvert_context_base::get_property_list()
    -> std::vector<tcamprop_system::property_desc>
{
    return prop_list_.get_property_list();
}

auto tcamconvert::tcamconvert_context_base::find_property(std::string_view name)
    -> tcamprop_system::property_interface*
{
    return prop_list_.find_property(name);
}

static auto make_wb_params(bool apply, auto_alg::wb_channel_factors factors) noexcept
{
    return img_filter::whitebalance_params { apply, factors.r, factors.g, factors.b, factors.g };
}

void tcamconvert::tcamconvert_context_base::transform(const img::img_descriptor& src,
                                                      const img::img_descriptor& dst)
{

    if (transform_binary_color_func_)
    {
        inspect_image(src);

        img_filter::filter_params tmp = {
            make_wb_params(apply_wb_, wb_channels_),
        };

        transform_binary_color_func_(dst, src, tmp);
    }
    else
    {
        transfrom_binary_mono_func_(dst, src);
    }
}

void tcamconvert::tcamconvert_context_base::filter(const img::img_descriptor& src)
{
    if (transform_unary_func_)
    {
        inspect_image(src);

        transform_unary_func_(src, make_wb_params(apply_wb_, wb_channels_));
    }
}

void tcamconvert::tcamconvert_context_base::inspect_image(const img::img_descriptor& src)
{
    auto_alg::auto_pass_params params = {};

    params.frame_number = frame_number_++;
    params.time_point = std::chrono::duration_cast<std::chrono::microseconds>(
                            std::chrono::steady_clock::now().time_since_epoch())
                            .count();

    params.wb.is_software_whitebalance = true;
    params.wb.auto_enabled = wb_auto_;
    if (wb_once_)
    {
        params.wb.one_push_enabled = true;
    }
    params.wb.channels = wb_channels_;

    auto res = auto_alg::auto_pass(*auto_pass_state_, src, params);
    if (res.wb.wb_changed)
    {
        wb_channels_ = res.wb.channels;
        if (wb_once_)
        {
            wb_once_ = res.wb.one_push_still_running;
        }
    }
}

auto tcamconvert::tcamconvert_context_base::get_transform_mode() const noexcept
    -> tcamconvert::tcamconvert_context_base::transform_mode
{
    if (src_type_.empty())
    {
        return transform_mode::binary;
    }
    return src_type_.fourcc_type() == dst_type_.fourcc_type() ? transform_mode::unary :
                                                                transform_mode::binary;
}

auto tcamconvert::tcamconvert_context_base::get_color_mode() const noexcept
    -> tcamconvert::tcamconvert_context_base::color_mode
{
    if (src_type_.empty())
    {
        return color_mode::bayer;
    }
    return img::is_mono_fcc(src_type_.fourcc_type()) ? color_mode::mono : color_mode::bayer;
}
