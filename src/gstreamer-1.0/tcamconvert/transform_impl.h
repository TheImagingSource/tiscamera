#pragma once

#include "../../../libs/dutils_image/src/dutils_img_filter/dutils_img_filter_params.h"

#include <dutils_img/dutils_img.h>
#include <functional>
#include <vector>

namespace tcamconvert
{
auto tcamconvert_get_all_input_fccs() -> std::vector<img::fourcc>;
auto tcamconvert_get_all_output_fccs() -> std::vector<img::fourcc>;
auto tcamconvert_get_supported_input_fccs(img::fourcc src_fcc) -> std::vector<img::fourcc>;
auto tcamconvert_get_supported_output_fccs(img::fourcc src_fcc) -> std::vector<img::fourcc>;

using transform_unary_wb_func = void (*)(const img::img_descriptor& dst,
                                         const img_filter::whitebalance_params& params);


using transform_binary_func =
    std::function<void(const img::img_descriptor& dst, const img::img_descriptor& src)>;
using transform_binary_wb_func = std::function<void(const img::img_descriptor& dst,
                                                    const img::img_descriptor& src,
                                                    img_filter::filter_params& params)>;


struct transform_context
{
    bool setup(img::img_type src_type, img::img_type dst_type);

    void transform(const img::img_descriptor& src,
                   const img::img_descriptor& dst,
                   const img_filter::whitebalance_params& params);
    void filter(const img::img_descriptor& src, const img_filter::whitebalance_params& params);

private:
    transform_unary_wb_func transform_unary_wb_func_ = nullptr;
    transform_binary_func transfrom_binary_mono_func_;
    transform_binary_wb_func transform_fccXX_to_dst_func_;

private: // byXX -> bgra stuff
    std::vector<uint8_t> transform_intermediate_buffer_;
};
} // namespace tcamconvert
