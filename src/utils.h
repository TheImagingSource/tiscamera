/*
 * Copyright 2014 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TCAM_UTILS_H
#define TCAM_UTILS_H

#include "base_types.h"
#include "compiler_defines.h"

#include <memory>
#include <string>
#include <vector>

VISIBILITY_INTERNAL

namespace tcam
{

std::string propertyType2String(TCAM_PROPERTY_TYPE);


std::vector<std::string> split_string(const std::string& to_split, const std::string& delim);


/**
 * @brief Check id bit is set.
 * @param value    - bitfield that shall be checked
 * @param bitindex - index that shall be checked
 * @return true if bit is 1
 */
inline bool is_bit_set(unsigned int value, unsigned int bitindex)
{
    return (value & (1 << bitindex)) != 0;
}


/**
 * @brief Set bit
 * @param value    - bitfield that shall be manipulated
 * @param bitindex - index of the bit that shall be set
 * @return the manipulated bitfield
 */
inline unsigned int set_bit(unsigned int value, unsigned int bitindex)
{
    return (value |= (1 << bitindex));
}


/**
 * @brief Unset bit
 * @param value    - bitfield that shall be manipulated
 * @param bitindex - index of the bit that shall be unset
 * @return the manipulated bitfield
 */
inline unsigned int unset_bit(unsigned int value, unsigned int bitindex)
{
    return (value &= ~(1 << bitindex));
}


int tcam_xioctl(int fd, unsigned int request, void* arg);

unsigned int tcam_get_required_buffer_size(const struct tcam_video_format* format);


/**
 * @brief Create framerate list for range
 * @param min - minimum framerate
 * @param max - maximum framerate
 * @return vector containing all step from min to max; empty on error
 */
std::vector<double> create_steps_for_range(double min, double max);

/**
 * @brief Calculate required image size
 * @param width  - width of the image
 * @param height - height of the image
 * @param fourcc - format description
 * @return required buffer size in byte
 */
uint64_t get_buffer_length(unsigned int width, unsigned int height, uint32_t fourcc);

/**
 * Description for get_pitch_length.
 * @param width  - pixel width
 * @param fourcc - pixel format
 * @return row length of image in byte
 */
uint32_t get_pitch_length(unsigned int width, uint32_t fourcc);

/**
 * Check if buffer has correct length
 * @param buffer that shall be checked
 * @return true if buffer has correct length
 */
bool is_buffer_complete(const struct tcam_image_buffer* buffer);

/**
 * @name calculate_auto_center
 * @param sensor - size of the sensor on which image shall be centered
 * @param image  - image size of the image that shall be auto centered
 * @return coordinates that shall be used for offsets
 */
tcam_image_size calculate_auto_center(const tcam_image_size& sensor, const tcam_image_size& image);


bool compare_double(double val1, double val2);


bool are_equal(const tcam_image_size& s1, const tcam_image_size& s2);

bool in_range(const tcam_image_size& minimum,
              const tcam_image_size& maximum,
              const tcam_image_size& value);

bool are_equal(const struct tcam_resolution_description& res1,
               const struct tcam_resolution_description& res2);


bool are_equal(const struct tcam_video_format_description& res1,
               const struct tcam_video_format_description& res2);

bool is_smaller(const tcam_image_size& s1, const tcam_image_size& s2);


/**
 * @brief generate new property ids
 * @return new unique property id
 */
TCAM_PROPERTY_ID generate_unique_property_id();

unsigned int get_pid_from_lockfile(const std::string& filename);

bool is_process_running(unsigned int pid);


/**
 * @brief map value from the input range to value in the output range
 * @param input_start - smallest value input can have
 * @param input_end - largest value input can have
 * @param output_start - smallest value output can have
 * @param output_end - largest value output can have
 * @param value - value that shall be mapped
 *
 * @return value in output range
 */
double map_value_ranges(double input_start,
                        double input_end,
                        double output_start,
                        double output_end,
                        double value);


/**
 * @brief retrieve environment variable
 * @param name - name of the variable that shall be retrieved
 * @param backup - value that shall be returned when name is not set
 *
 * @return value of the environment variable or backup
 */
std::string get_environment_variable(const std::string& name, const std::string& backup);

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_UTILS_H */
