#pragma once

#include "../../lib/dutils_image/src/dutils_img_filter/transform/transform_base.h" // img_filter::transform_function_type
#include "tcamprop_provider_funcbased.h"

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

    // Inherited via property_interface
    virtual std::vector<tcamprop_system::property_desc> get_property_list() final;

    virtual tcamprop_system::property_interface* find_property(std::string_view name) final;


    void transform(const img::img_descriptor& src, const img::img_descriptor& dst);
    void filter(const img::img_descriptor& src);

private:
    img_filter::transform_function_type transfrom_binary_mono_func_ = nullptr;

    std::function<void(const img::img_descriptor& dst,
                       const img::img_descriptor& src,
                       img_filter::filter_params& params)>
        transform_binary_color_func_;

    wb_func transform_unary_func_ = nullptr;

private:
    void inspect_image(const img::img_descriptor& src);

    enum class color_mode
    {
        mono,
        bayer,
    };
    enum class transform_mode
    {
        unary,
        binary,
    };

    color_mode get_color_mode() const noexcept;
    transform_mode get_transform_mode() const noexcept;

    tcamprop_system::property_list_funcbased prop_list_;

    auto_alg::state_ptr auto_pass_state_ = auto_alg::make_state_ptr();
    auto_alg::auto_pass_params auto_alg_params_ = {};

    bool apply_wb_ = true;
    bool wb_auto_ = true;
    bool wb_once_ = false;

    auto_alg::wb_channel_factors wb_channels_ = {};

    int64_t frame_number_ = 0;
};
} // namespace tcamconvert
