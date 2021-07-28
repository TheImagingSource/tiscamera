#pragma once

#include <dutils_img/dutils_img.h>
#include <gst/gst.h>

#include <optional>
#include <string>
#include <vector>
#include "gst_ptr.h"

namespace gst_helper
{
auto get_gst_struct_image_dim(const GstStructure& structure) -> std::optional<img::dim>;
auto get_gst_struct_fcc(const GstStructure& structure) -> img::fourcc;

auto get_gst_struct_image_type(const GstStructure& structure) -> img::img_type;
auto get_gst_struct_framerate(const GstStructure& structure) -> std::optional<double>;

void set_gst_struct_framerate(GstStructure& structure, double framerate) noexcept;

auto generate_caps_with_dim( const std::vector<img::fourcc>& fcc_list ) -> gst_ptr<GstCaps>;

} // namespace gst_helper
