

#ifndef CTRL_MULTIMEDIA_H
#define CTRL_MULTIMEDIA_H

#include <string>
#include <tis.h>

namespace tis_imaging
{

bool save_image (Grabber& g, const std::string& filename);

bool save_stream (Grabber& g, const std::string& file);

} /* namespace tis_imaging */

#endif /* CTRL_MULTIMEDIA_H */










