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
#include <optional>


VISIBILITY_INTERNAL

namespace tcam
{

std::vector<std::string> split_string(const std::string& to_split, const std::string& delim);


int tcam_xioctl(int fd, unsigned int request, void* arg);


/**
 * @brief Create framerate list for range
 * @param min - minimum framerate
 * @param max - maximum framerate
 * @return vector containing all step from min to max; empty on error
 */
std::vector<double> create_steps_for_range(double min, double max);

/**
 * @name calculate_auto_center
 * @param sensor - size of the sensor on which image shall be centered
 * @param step - step size the sensor permits
 * @param image - image size of the image that shall be auto centered
 * @param scale - scaling configuration that is used
 * @return coordinates that shall be used for offsets
 */
tcam_image_size calculate_auto_center(const tcam_image_size& sensor,
                                      const tcam_image_size& step,
                                      const tcam_image_size& image,
                                      const image_scaling& scale);


bool compare_double(double val1, double val2);


bool in_range(const tcam_image_size& minimum,
              const tcam_image_size& maximum,
              const tcam_image_size& value);

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
 * @brief Check if environment variable is set
 * @param name - string containing the name of the environment variable to check
 * @return bool, true if environment variable is set
 */
bool is_environment_variable_set (const std::string& name);

    /**
 * @brief retrieve environment variable
 * @param name - name of the variable that shall be retrieved
 * @param backup - value that shall be returned when name is not set
 *
 * @return value of the environment variable or backup
 */
std::string get_environment_variable(const std::string& name, const std::string& backup);

std::optional<int> get_environment_variable_int(const std::string& name);

/**
 * @brief set the thread name (for debuggers) of the specified thread.
 * @param thrd the thread specified.
 * @param name the name to set
 *
 * @return 0 on success, otherwise the error value returned by pthread_setname_np
 */
int set_thread_name(const char* name, pthread_t thrd = pthread_self());
int set_thread_name(const std::string& name, pthread_t thrd = pthread_self());

} /* namespace tcam */

VISIBILITY_POP

#endif /* TCAM_UTILS_H */
