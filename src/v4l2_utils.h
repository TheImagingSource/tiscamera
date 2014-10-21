

#ifndef V4L2_UTILS_H
#define V4L2_UTILS_H

#include "Properties.h"

namespace tis_imaging
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
 * @name tis_get_camera_count
 * @return number of available usb devices
 */
int tis_get_usb_camera_count ();


/**
 * @name
 * @param ptr        - pointer to the array that shall be filled
 * @param array_size - size of array that ptr points to
 * @return number of devices copied to ptr; -1 on error
 */
int tis_get_usb_camera_list (struct tis_device_info* ptr, unsigned int array_size);


} /* namespace tis_imaging */

#endif /* V4L2_UTILS_H */
