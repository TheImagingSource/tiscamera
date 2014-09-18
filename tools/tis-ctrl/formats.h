

#ifndef FORMATS_H
#define FORMATS_H

#include "tis.h"

#include <vector>
#include <string>

using namespace tis_imaging;

void list_formats (const std::vector<VideoFormatDescription>& available_formats);

void print_active_format (const VideoFormat& format);

bool set_active_format (Grabber& g, const std::string& new_format);

#endif /* FORMATS_H */
