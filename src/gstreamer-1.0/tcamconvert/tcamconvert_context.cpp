/*
 * Copyright 2021 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "tcamconvert_context.h"

#include "../../lib/dutils_image/src/dutils_img_filter/filter/whitebalance/wb_apply.h"
#include "../../lib/dutils_image/src/dutils_img_filter/transform/fcc1x_packed/fcc1x_packed_to_fcc.h"
#include "../../lib/dutils_image/src/dutils_img_filter/transform/fcc1x_packed/transform_fcc1x_to_fcc8.h"
#include "../../lib/dutils_image/src/dutils_img_filter/transform/fcc8_fcc16/transform_fcc8_fcc16.h"

#include <Tcam-0.1.h>
#include <cassert>
#include <gst-helper/gst_element_chain.h>
#include <spdlog/spdlog.h>
#include <tcamprop_system/tcamprop_consumer.h>

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

static constexpr const char* BalanceWhiteRed_name = "BalanceWhiteRed";
static constexpr const char* BalanceWhiteGreen_name = "BalanceWhiteGreen";
static constexpr const char* BalanceWhiteBlue_name = "BalanceWhiteBlue";

} // namespace

tcamconvert::tcamconvert_context_base::tcamconvert_context_base(GstTCamConvert* self)
    : self_reference_(self)
{

}

bool tcamconvert::tcamconvert_context_base::setup(img::img_type src_type, img::img_type dst_type)
{
    auto clr_mode = img::is_mono_fcc(src_type.fourcc_type()) ? color_mode::mono : color_mode::bayer;

    transform_unary_func_ = nullptr;
    transfrom_binary_mono_func_ = nullptr;
    transform_binary_color_func_ = nullptr;

    if (src_type.fourcc_type() == dst_type.fourcc_type())
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

    return true;
}

void tcamconvert::tcamconvert_context_base::init_from_source()
{
    auto prop_elem = tcamprop_system::to_TcamProp(src_element_ptr_.get());
    assert(prop_elem != nullptr);

    bool val = tcamprop_system::has_property(
        prop_elem, "ClaimBalanceWhiteSoftware", tcamprop_system::prop_type::boolean);
    if (val)
    {
        apply_wb_ = tcamprop_system::set_value(prop_elem, "ClaimBalanceWhiteSoftware", true);
    }

    init_from_source_done_ = true;
}

auto tcamconvert::tcamconvert_context_base::get_property_list() -> std::vector<std::string_view>
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
        update_balancewhite_values_from_source();

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
        update_balancewhite_values_from_source();

        transform_unary_func_(src, make_wb_params(apply_wb_, wb_channels_));
    }
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


void tcamconvert::tcamconvert_context_base::update_balancewhite_values_from_source()
{
    if (!src_element_ptr_) {
        return;
    }

    auto prop_elem = tcamprop_system::to_TcamProp(src_element_ptr_.get());
    assert(prop_elem);
    if (!prop_elem) {
        return;
    }

    auto_alg::wb_channel_factors factors = wb_channels_;
    auto read_chan = [prop_elem](const char* name, float& val)
    {
        if (auto res = tcamprop_system::get_value<double>(prop_elem, name); res)
        {
            val = res.value();
        }
    };
    read_chan(BalanceWhiteRed_name, factors.r);
    read_chan(BalanceWhiteGreen_name, factors.g);
    read_chan(BalanceWhiteBlue_name, factors.b);

    wb_channels_ = factors;
}


static bool is_compatible_source_element(GstElement& element)
{
    if (!TCAM_IS_PROP(&element))
    { // When the element has no tcamprop interface, we can quit here
        return false;
    }
    return true;
}


bool tcamconvert::tcamconvert_context_base::try_connect_to_source(bool force)
{
    auto camera_src_ptr = gst_helper::find_upstream_element(*GST_ELEMENT(self_reference_),
                                                            is_compatible_source_element);
    if (camera_src_ptr == nullptr)
    {
        if (force)
        {
            // if we are connected, we look for our GstTcamSrc element, and only here whine about not finding it
            GST_ERROR_OBJECT(self_reference_,
                             "Unable to find a 'The Imaging Source' device. "
                             "tcamconvert can only be used in conjunction with such a device.");
        }
        return false;
    }
    // check if we already are attached to the newly found device
    if (camera_src_ptr.get() == src_element_ptr_.get())
    {
        return true;
    }

    const bool has_device_open =
        gst_helper::has_signal(G_OBJECT(camera_src_ptr.get()), "device-open");
    if (has_device_open)
    {
        assert(gst_helper::has_signal(G_OBJECT(camera_src_ptr.get()), "device-close"));

        bool res =
            signal_handle_device_open_.connect(G_OBJECT(camera_src_ptr.get()),
                                               "device-open",
                                               [this](GstElement*) { this->on_device_opened(); });
        assert(res || "Failed to register 'device-open' signal");
        bool res2 =
            signal_handle_device_close_.connect(G_OBJECT(camera_src_ptr.get()),
                                                "device-close",
                                                [this](GstElement*) { this->on_device_closed(); });
        assert(res2 || "Failed to register 'device-open' signal");
    }
    else
    {
        GST_ERROR_OBJECT(
            self_reference_,
            "Source element does not have 'device-open'/'device-close' events. Failing connect");
        return false;
    }
    src_element_ptr_ = std::move(camera_src_ptr);

    const auto cur_state = gst_helper::get_gststate(*src_element_ptr_, false);
    if (cur_state && cur_state.value() >= GST_STATE_READY)
    {
        init_from_source();
    }
    return true;
}

void tcamconvert::tcamconvert_context_base::on_device_opened()
{
    init_from_source();
}

void tcamconvert::tcamconvert_context_base::on_device_closed()
{
    init_from_source_done_ = false;
}

void tcamconvert::tcamconvert_context_base::on_input_pad_linked()
{
    try_connect_to_source(false);
}

void tcamconvert::tcamconvert_context_base::on_input_pad_unlinked()
{
    on_device_closed();
    signal_handle_device_open_.disconnect();
    signal_handle_device_close_.disconnect();
    src_element_ptr_ = nullptr;
}
