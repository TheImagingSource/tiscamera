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

#include <cassert>
#include <gst-helper/gstelement_helper.h>
#include <tcamprop1.0_consumer/tcamprop1_consumer.h>

namespace
{

static constexpr const char* ClaimBalanceWhiteSoftware_name = "ClaimBalanceWhiteSoftware";
static constexpr const char* BalanceWhiteRed_name = "BalanceWhiteRed";
static constexpr const char* BalanceWhiteGreen_name = "BalanceWhiteGreen";
static constexpr const char* BalanceWhiteBlue_name = "BalanceWhiteBlue";

} // namespace

tcamconvert::tcamconvert_context_base::tcamconvert_context_base(GstTCamConvert* self)
    : self_reference_(self)
{
}

void tcamconvert::tcamconvert_context_base::init_from_source()
{
    whitebalance_params_.apply = false;

    auto prop_elem = tcamprop1_consumer::get_TcamPropertyProvider(*src_element_ptr_);
    assert(prop_elem != nullptr);
    if (!prop_elem)
    {
        return;
    }

    tcamprop1_consumer::TcamPropertyProvider_wrapper provider(*prop_elem);

    auto p_claim_ptr = provider.get_property_ptr<tcamprop1::property_interface_boolean>(
        "ClaimBalanceWhiteSoftware");
    if (p_claim_ptr)
    {
        auto err = p_claim_ptr->set_property_value(true);
        if (!err)
        {
            auto wb_red = provider.get_property_ptr<tcamprop1::property_interface_float>(
                BalanceWhiteRed_name);
            auto wb_green = provider.get_property_ptr<tcamprop1::property_interface_float>(
                BalanceWhiteGreen_name);
            auto wb_blue = provider.get_property_ptr<tcamprop1::property_interface_float>(
                BalanceWhiteBlue_name);

            assert(wb_red && wb_green && wb_blue);
            if (wb_red && wb_green && wb_blue)
            {
                whitebalance_params_.apply = true;
                wb_red_ = std::move(wb_red);
                wb_green_ = std::move(wb_green);
                wb_blue_ = std::move(wb_blue);
            }
        }
    }

    init_from_source_done_ = true;
}

auto tcamconvert::tcamconvert_context_base::fetch_balancewhite_values_from_source()
    -> img_filter::whitebalance_params
{
    if (!src_element_ptr_)
    {
        return {};
    }

    auto prop_elem = tcamprop1_consumer::get_TcamPropertyProvider(*src_element_ptr_);
    assert(prop_elem);
    if (!prop_elem)
    {
        return {};
    }

    if (!whitebalance_params_.apply)
    {
        return {};
    }

    auto factors = whitebalance_params_;

    auto read_chan = [prop_elem](auto& ptr, float& val)
    {
        if (!ptr)
            return;
        if (auto res = ptr->get_property_value(); res)
        {
            val = res.value();
        }
    };
    read_chan(wb_red_, factors.wb_rr);
    read_chan(wb_green_, factors.wb_gr);
    read_chan(wb_blue_, factors.wb_bb);
    factors.wb_gb = factors.wb_gr;

    return whitebalance_params_ = factors;
}


static bool is_compatible_source_element(GstElement& element)
{
    if (!TCAM_IS_PROPERTY_PROVIDER(&element))
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
        if (!res)
        {
            GST_ERROR_OBJECT(self_reference_, "Failed to register 'device-open' signal");
            return false;
        }
        bool res2 =
            signal_handle_device_close_.connect(G_OBJECT(camera_src_ptr.get()),
                                                "device-close",
                                                [this](GstElement*) { this->on_device_closed(); });
        if (!res2)
        {
            GST_ERROR_OBJECT(self_reference_, "Failed to register 'device-close' signal");
            return false;
        }
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
    wb_red_.reset();
    wb_green_.reset();
    wb_blue_.reset();
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

bool tcamconvert::tcamconvert_context_base::setup(img::img_type src_type, img::img_type dst_type)
{
    if (trans_impl_.setup(src_type, dst_type))
    {
        this->src_type_ = src_type;
        this->dst_type_ = dst_type;
        return true;
    }

    return false;
}

void tcamconvert::tcamconvert_context_base::transform(const img::img_descriptor& src,
                                                      const img::img_descriptor& dst)
{
    trans_impl_.transform(src, dst, fetch_balancewhite_values_from_source());
}

void tcamconvert::tcamconvert_context_base::filter(const img::img_descriptor& src)
{
    trans_impl_.filter(src, fetch_balancewhite_values_from_source());
}
