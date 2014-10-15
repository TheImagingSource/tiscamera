
#ifndef _TIS_UTILS_H_
#define _TIS_UTILS_H_
 
#include "base_types.h"
#include "Property.h"

#include <string>
#include <vector>
#include <memory>
 
namespace tis_imaging
{

std::string propertyType2String (PROPERTY_TYPE);

std::string fourcc2string (uint32_t fourcc);

uint32_t string2fourcc (const std::string& s);


std::vector<std::string> split_string (const std::string& to_split, const std::string& delim);


inline bool is_bit_set (unsigned int value, unsigned int bitindex)
{
    return (value & (1 << bitindex)) != 0;
}


inline unsigned int set_bit (unsigned int value, unsigned int bitindex)
{
    return (value |= (1 << bitindex));
}


inline unsigned int unset_bit (unsigned int value, unsigned int bitindex)
{
    return (value &= ~(1 << bitindex));
}


int tis_xioctl (int fd, int request, void* arg);

unsigned int tis_get_required_buffer_size (struct tis_video_format* format);

const char* fourcc2description (uint32_t fourcc);

uint32_t description2fourcc (const char* description);



/**
 * Create step list for given range
 * @return vector containing all step from min to max; empty on error
 */
std::vector<double> createStepsForRange (double min, double max);

/**
 * @return required buffer size in byte
 */
uint64_t getBufferLength (unsigned int width, unsigned int height, uint32_t fourcc);

/**
 * @return row length of image in byte
 */
uint32_t getPitchLength (unsigned int width, uint32_t fourcc);


/**
 * @name calculateAutoCenter
 * @param sensor - size of the sensor on which image shall be centered
 * @param image  - image size of the image that shall be auto centered
 * @return coordinates that shall be used for offsets
 */
IMG_SIZE calculateAutoCenter (const IMG_SIZE& sensor, const IMG_SIZE& image);


std::shared_ptr<Property> find_property (std::vector<std::shared_ptr<Property>>& properties, const std::string& property_name);


} /* namespace tis_imaging */

#endif /* _TIS_UTILS_H_ */
