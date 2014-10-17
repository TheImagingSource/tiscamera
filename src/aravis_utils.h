

#ifndef ARAVIS_UTILS_H
#define ARAVIS_UTILS_H

#include "Properties.h"

#include "arv.h"

namespace tis_imaging
{

std::shared_ptr<Property> createProperty (ArvCamera* camera,
                                          ArvGcNode* node,
                                          std::shared_ptr<PropertyImpl> impl);

}; /* namespace tis_imaging */

#endif /* ARAVIS_UTILS_H */
