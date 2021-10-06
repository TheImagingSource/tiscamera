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

#pragma once

#include "transform_impl.h"

#include <dutils_img/dutils_img.h>
#include <dutils_img_pipe/auto_alg_pass.h>
#include <functional>
#include <gst-helper/gst_signal_helper.h>
#include <gst-helper/helper_functions.h>
#include <tcamprop1.0_base/tcamprop_property_interface.h>

struct GstTCamConvert;

namespace tcamconvert
{
class tcamconvert_context_base
{
public:
    img::img_type src_type_;
    img::img_type dst_type_;

    void on_input_pad_linked();
    void on_input_pad_unlinked();

public:
    tcamconvert_context_base(GstTCamConvert* self);

    bool setup(img::img_type src_type, img::img_type dst_type);

    void transform(const img::img_descriptor& src, const img::img_descriptor& dst);
    void filter(const img::img_descriptor& src);

    bool try_connect_to_source(bool force);

private:
    img_filter::whitebalance_params whitebalance_params_;

    transform_context trans_impl_;

    auto fetch_balancewhite_values_from_source() -> img_filter::whitebalance_params;

private:
    void init_from_source();
    bool init_from_source_done_ = false;

    void on_device_opened();
    void on_device_closed();

    gst_helper::gst_device_connect_signal signal_handle_device_open_;
    gst_helper::gst_device_connect_signal signal_handle_device_close_;

    gst_helper::gst_ptr<GstElement> src_element_ptr_;
    std::unique_ptr<tcamprop1::property_interface_float>    wb_red_;
    std::unique_ptr<tcamprop1::property_interface_float>    wb_green_;
    std::unique_ptr<tcamprop1::property_interface_float>    wb_blue_;

    GstTCamConvert* self_reference_ = nullptr;
};
} // namespace tcamconvert
