
#pragma once

/**
 * Helper for dutils img::fourcc/img::img_type and gstreamer caps interop
 */

#include <dutils_img/dutils_img.h>
#include <gst/gst.h>

#include <optional>
#include <string>
#include <vector>
#include "gst_ptr.h"

namespace gst_helper
{
/** Gets the "width"/"height" contents of the structure as a img::dim.
 * @return null_opt when anything failed (fields are ranges, or not present ...)
 */
auto get_gst_struct_image_dim(const GstStructure& structure) -> std::optional<img::dim>;
/** Gets the struct-name/"format" field contents of the structure as a img::fourcc
 * @return img::FCC_NULL if the field is not present, is a list or when there is no equivalent GstCaps-format to img::fourcc mapping.
 */
auto get_gst_struct_fcc(const GstStructure& structure) -> img::fourcc;
/** Gets the dim/fcc struct contents and returns a img::img_type for that.
 * @return Either a valid img_type or a img_type with empty() == true when no img_type could be created from the structure
 */
auto get_gst_struct_image_type(const GstStructure& structure) -> img::img_type;
/** Gets the dim/fcc struct contents of the first strcut fround in the caps and returns a img::img_type for that.
 * @return Either a valid img_type or a img_type with empty() == true when no img_type could be created from the structure
 */
auto get_img_type_from_fixated_gstcaps( const GstCaps& structure )->img::img_type;
/** Gets the "framerate" field contents of the structure as a double.
 * @return null_opt when anything failed (field is a ranges, or not present ...)
 */
auto get_gst_struct_framerate(const GstStructure& structure) -> std::optional<double>;

/** Sets the "framerate" field of the structure to this framerate.
 */
void set_gst_struct_framerate(GstStructure& structure, double framerate) noexcept;

/** Creates a GstCaps object from the fcc_list parameter. width and height are full range fields. "[ 1, 2147483647 ]"
 * Example:
        auto caps = generate_caps_with_dim( { img::fourcc::BGGR, img::fourcc::BGGR16 } ):
 * Generates this caps struct:
       video/x-bayer
                 format: { (string)bggr, (string)bggr16 }
                  width: [ 1, 2147483647 ]
                 height: [ 1, 2147483647 ]
*/
auto generate_caps_with_dim( const std::vector<img::fourcc>& fcc_list ) -> gst_ptr<GstCaps>;

auto convert_GstCaps_to_fcc_list( const GstCaps& caps )->std::vector<img::fourcc>;
auto convert_GstStructure_to_fcc_list( const GstStructure& strct )->std::vector<img::fourcc>;

} // namespace gst_helper
