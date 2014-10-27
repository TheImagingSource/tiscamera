


#ifndef SERIALIZATION_H
#define SERIALIZATION_H


#include "Grabber.h"

#include <string>

namespace tcam
{

/**
 * @brief store given list of capture devices in the specified file
 * @param[in] device_list - devices that shall be stored as xml
 * @param[in] filename - file in which the created xml structure shall be stored
 * @return true on success, else false
 */
bool export_device_list (const std::vector<CaptureDevice>& device_list,
                         const std::string& filename);


bool load_xml_description (const std::string& filename,
                           const CaptureDevice& device,
                           VideoFormat& format,
                           std::vector<std::shared_ptr<Property>>& properties);

bool save_xml_description (const std::string& filename,
                           const CaptureDevice& device,
                           const VideoFormat& format,
                           const std::vector<std::shared_ptr<Property>>& properties);

} /* namespace tcam */

#endif /* SERIALIZATION_H */
