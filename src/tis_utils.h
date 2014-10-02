
#ifndef _TIS_UTILS_H_
#define _TIS_UTILS_H_

// #include "tis_video.h"

#include "base_types.h"

#include <string>
#include <vector>

namespace tis_imaging
{

std::string propertyType2String (const PROPERTY_TYPE&);

std::string fourcc2string (const uint32_t& fourcc);

uint32_t string2fourcc (const std::string& s);


std::vector<std::string> split_string (const std::string& to_split, const std::string &delim);


int tis_xioctl (int fd, int request, void* arg);

unsigned int tis_get_required_buffer_size (struct tis_video_format* format);

const char* fourcc2description (const uint32_t& fourcc);

uint32_t description2fourcc (const char* description);



/**
 * Create step list for given range
 * @return vector containing all step from min to max; empty on error
 */
std::vector<double> createStepsForRange (double min, double max);

/**
 * @return required buffer size in byte
 */
uint64_t getBufferLength (unsigned int width, unsigned int height, unsigned int fourcc);

} /* namespace tis_imaging */

#endif /* _TIS_UTILS_H_ */
