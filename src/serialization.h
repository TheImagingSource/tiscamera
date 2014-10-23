


#ifndef SERIALIZATION_H
#define SERIALIZATION_H


#include "Grabber.h"

#include <string>

namespace tis_imaging
{

bool load_xml_description (const std::string& filename,
                           const CaptureDevice& device,
                           VideoFormat& format,
                           std::vector<std::shared_ptr<Property>>& properties);

bool save_xml_description (const std::string& filename,
                           const CaptureDevice& device,
                           const VideoFormat& format,
                           const std::vector<std::shared_ptr<Property>>& properties);

} /* namespace tis_imaging */

#endif /* SERIALIZATION_H */
