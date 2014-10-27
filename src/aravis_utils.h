

#ifndef ARAVIS_UTILS_H
#define ARAVIS_UTILS_H

#include "Properties.h"

#include "arv.h"

#include "DeviceInfo.h"

namespace tcam
{

std::shared_ptr<Property> createProperty (ArvCamera* camera,
                                          ArvGcNode* node,
                                          std::shared_ptr<PropertyImpl> impl);


std::vector<DeviceInfo> get_aravis_device_list ();

}; /* namespace tcam */

#endif /* ARAVIS_UTILS_H */
