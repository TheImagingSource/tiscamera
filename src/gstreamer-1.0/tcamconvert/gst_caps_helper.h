#pragma once

#include <dutils_img/dutils_img.h>
#include <gst/gst.h>
#include <memory>
#include <optional>
#include <string>

namespace gst_helper
{
std::optional<img::dim> get_gst_struct_image_dim(const GstStructure* structure);
img::fourcc get_gst_struct_fcc(const GstStructure* structure);

img::img_type get_gst_struct_image_type(const GstStructure* structure);
std::optional<double> get_gst_struct_framerate(const GstStructure* structure);

void set_gst_struct_framerate(GstStructure* structure, double framerate) noexcept;

std::string caps_to_string(const GstCaps* structure);
} // namespace gst_helper
