

#ifndef FORMATS_H
#define FORMATS_H

#include "tis.h"

#include <vector>
#include <string>

using namespace tcam;


/**
 * @brief print function for VideoFormatDescriptions
 * @param available_formats - format descriptions that shell be printed
 */
void list_formats (const std::vector<VideoFormatDescription>& available_formats);


/**
 * @brief print function for VideoFormat
 * @param format - VideoFormat that shall be printed
 */
void print_active_format (const VideoFormat& format);


/**
 * @brief Set video format for device
 * @param g - Grabber of the device which shall be used
 * @param new_format - string describing the format that shall be set
 * @return true on success
 */
bool set_active_format (Grabber& g, const std::string& new_format);

#endif /* FORMATS_H */
