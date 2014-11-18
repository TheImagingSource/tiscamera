

#ifndef ARAVIS_UTILS_H
#define ARAVIS_UTILS_H

#include "Properties.h"

#include "arv.h"

#include "DeviceInfo.h"

#include "compiler_defines.h"

VISIBILITY_INTERNAL

namespace tcam
{

std::shared_ptr<Property> createProperty (ArvCamera* camera,
                                          ArvGcNode* node,
                                          std::shared_ptr<PropertyImpl> impl);


std::vector<DeviceInfo> get_aravis_device_list ();

}; /* namespace tcam */

VISIBILITY_POP

#endif /* ARAVIS_UTILS_H */
