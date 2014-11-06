

#ifndef CTRL_MULTIMEDIA_H
#define CTRL_MULTIMEDIA_H

#include <string>
#include <tcam.h>

namespace tcam
{

bool save_image (CaptureDevice& g, const std::string& filename);

bool save_stream (CaptureDevice& g, const std::string& file);

} /* namespace tcam */

#endif /* CTRL_MULTIMEDIA_H */
