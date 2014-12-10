

#ifndef TCAM_V4L2_UTILS_H
#define TCAM_V4L2_UTILS_H

#include "Properties.h"
#include "DeviceInfo.h"

#include <linux/videodev2.h>

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

uint32_t convertV4L2flags (uint32_t v4l2_flags);

/**
 * @brief Create Property and return shared_ptr to base class
 *
 * @param fd - file descriptor for responsible device
 * @param queryctrl
 * @param ctrl
 * @param impl - shared_ptr of the responsible implementation
 *
 * @return shared_ptr to newly created Property; nullptr on failure
 */
std::shared_ptr<Property> createProperty (int fd,
                                          struct v4l2_queryctrl* queryctrl,
                                          struct v4l2_ext_control* ctrl,
                                          std::shared_ptr<PropertyImpl> impl);


/**
 * @name get_v4l2_device_list
 * @brief lists all supported v4l2 devices
 * @return vector containing all found v4l2 devices
 */
std::vector<DeviceInfo> get_v4l2_device_list ();

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_V4L2_UTILS_H */
