

#ifndef ARAVIS_UTILS_H
#define ARAVIS_UTILS_H

#include "Properties.h"

#include "arv.h"

#include "CaptureDevice.h"

namespace tis_imaging
{

std::shared_ptr<Property> createProperty (ArvCamera* camera,
                                          ArvGcNode* node,
                                          std::shared_ptr<PropertyImpl> impl);


/**
 * @name tis_get_camera_count
 * @return number of available gige devices
 */
int tis_get_gige_camera_count ();


/**
 * @name
 * @param ptr        - pointer to the array that shall be filled
 * @param array_size - size of array that ptr points to
 * @return number of devices copied to ptr; -1 on error
 */
int tis_get_gige_camera_list (struct tis_device_info* ptr, unsigned int array_size);


std::vector<CaptureDevice> get_aravis_device_list ();

}; /* namespace tis_imaging */

#endif /* ARAVIS_UTILS_H */
