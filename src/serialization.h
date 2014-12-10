


#ifndef TCAM_SERIALIZATION_H
#define TCAM_SERIALIZATION_H


#include "CaptureDevice.h"

#include <string>

namespace tcam
{

/**
 * @brief store given list of capture devices in the specified file
 * @param[in] device_list - devices that shall be stored as xml
 * @param[in] filename - file in which the created xml structure shall be stored
 * @return true on success, else false
 */
bool export_device_list (const std::vector<DeviceInfo>& device_list,
                         const std::string& filename);


bool load_xml_description (const std::string& filename,
                           const DeviceInfo& device,
                           VideoFormat& format,
                           std::vector<std::shared_ptr<Property>>& properties);

bool save_xml_description (const std::string& filename,
                           const DeviceInfo& device,
                           const VideoFormat& format,
                           const std::vector<std::shared_ptr<Property>>& properties);

} /* namespace tcam */

#endif /* TCAM_SERIALIZATION_H */
