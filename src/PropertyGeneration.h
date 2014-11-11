



#ifndef PROPERTYGENERATION_H_
#define PROPERTYGENERATION_H_

#include "Properties.h"

#include <memory>

namespace tcam
{


/**
 * @param props - vector of existing properties
 * @param impl - PropertyImpl that shall be used for new properties
 * @return vector containing the simulated properties; can be empty
 */
std::vector<std::shared_ptr<Property>> generate_simulated_properties (std::vector<std::shared_ptr<Property>> props,
                                                                      std::shared_ptr<PropertyImpl> impl);


/**
 * @param new_property
 * @param props - vector of device properties
 * @param sensor - maximum image size the sensor allows
 * @param current_format - current image size
 * @return true on success
 */
bool handle_auto_center (const Property& new_property,
                         std::vector<std::shared_ptr<Property>>& props,
                         const tcam_image_size& sensor,
                         const tcam_image_size& current_format);

} /* namespace tcam */

#endif /* PROPERTYGENERATION_H_ */
