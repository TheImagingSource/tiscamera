#pragma once

#include "../../lib/dutils_image/src/dutils_img_filter/transform/transform_base.h" // img_filter::transform_function_type
#include "../gst_helper/gst_helper.h"
#include "../tcamprop_system/tcamprop_provider_funcbased.h"

#include <dutils_img/dutils_img.h>
#include <dutils_img_pipe/auto_alg_pass.h>
#include <functional>

namespace tcamconvert
{
class tcamconvert_context_base : public tcamprop_system::property_list_interface
{
    using wb_func = void (*)(const img::img_descriptor& dst,
                             const img_filter::whitebalance_params& params);

public:
    img::img_type src_type_;
    img::img_type dst_type_;

public:
    tcamconvert_context_base();

    bool setup(img::img_type src_type, img::img_type dst_type);
    bool setup(const gst_helper::gst_ptr<GstElement>& src_element);

    void clear();

    // Inherited via property_interface
    virtual std::vector<std::string_view> get_property_list() final;

    virtual tcamprop_system::property_interface* find_property(std::string_view name) final;

    void transform(const img::img_descriptor& src, const img::img_descriptor& dst);

    void update_balancewhite_values_from_source();

    void filter( const img::img_descriptor& src );

private:
    img_filter::transform_function_type transfrom_binary_mono_func_ = nullptr;

    std::function<void(const img::img_descriptor& dst,
                       const img::img_descriptor& src,
                       img_filter::filter_params& params)>
        transform_binary_color_func_;

    wb_func transform_unary_func_ = nullptr;

private:
    enum class color_mode
    {
        mono,
        bayer,
    };

    color_mode get_color_mode() const noexcept;

    tcamprop_system::property_list_funcbased prop_list_;

    bool apply_wb_ = true;

    auto_alg::wb_channel_factors wb_channels_ = {};

    gst_helper::gst_ptr<GstElement> src_element_ptr_;
};
} // namespace tcamconvert
