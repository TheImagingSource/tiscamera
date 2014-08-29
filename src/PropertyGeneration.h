



#ifndef PROPERTYGENERATION_H_
#define PROPERTYGENERATION_H_

#include "Properties.h"

#include <memory>


#if HAVE_ARAVIS
#include <arv.h>
#endif


namespace tis_imaging 
{


std::shared_ptr<Property> createProperty (int fd,
                                          struct v4l2_queryctrl* queryctrl,
                                          struct v4l2_ext_control* ctrl,
                                          std::shared_ptr<PropertyImpl> impl);
#if HAVE_ARAVIS

std::shared_ptr<Property> createProperty (ArvCamera* camera,
                                          ArvGcNode* node,
                                          std::shared_ptr<PropertyImpl> impl);

#endif

} /* namespace tis_imaging */

#endif /* PROPERTYGENERATION_H_ */
