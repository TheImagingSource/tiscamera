



#ifndef PROPERTYGENERATION_H_
#define PROPERTYGENERATION_H_

#include "Properties.h"

#include <memory>


#if HAVE_ARAVIS
#include <arv.h>
#endif


namespace tis_imaging 
{


#if HAVE_ARAVIS

std::shared_ptr<Property> createProperty (ArvCamera* camera,
                                          ArvGcNode* node,
                                          std::shared_ptr<PropertyImpl> impl);

#endif

} /* namespace tis_imaging */

#endif /* PROPERTYGENERATION_H_ */
