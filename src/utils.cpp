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

#include "utils.h"

#include "logging.h"

//#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstring>
//#include <dutils_img/image_transform_base.h>
#include <errno.h>
#include <fstream>
#include <limits>
#include <pthread.h>
#include <signal.h> // kill
#include <sys/ioctl.h>

using namespace tcam;

std::vector<std::string> tcam::split_string(const std::string& to_split, const std::string& delim)
{
    std::vector<std::string> vec;

    size_t beg = 0;
    size_t end = 0;

    while (end != std::string::npos)
    {
        end = to_split.find_first_of(delim, beg);

        vec.push_back(to_split.substr(beg, end - beg));

        beg = end + delim.size();
    }

    return vec;
}


int tcam::tcam_xioctl(int fd, unsigned int request, void* arg)
{
    constexpr int IOCTL_RETRY = 4;

    int ret = 0;
    int tries = IOCTL_RETRY;
    do {
        ret = ioctl(fd, request, arg);
        // ret = v4l2_ioctl(fd, request, arg);
    } while (ret && tries-- && ((errno == EINTR) || (errno == EAGAIN) || (errno == ETIMEDOUT)));

    if (ret && (tries <= 0))
    {
        SPDLOG_ERROR("ioctl ({}) retried {} times - giving up: {})\n",
                     request,
                     IOCTL_RETRY,
                     strerror(errno));
    }

    return (ret);
}


std::vector<double> tcam::create_steps_for_range(double min, double max)
{
    std::vector<double> vec;

    if (max <= min)
        return vec;

    vec.push_back(min);

    // we do not want every framerate to have unnecessary decimals
    // e.g. 1.345678 instead of 1.00000
    double current_step = (int)min;

    // 0.0 is not a valid framerate
    if (current_step < 1.0)
        current_step = 1.0;

    while (current_step < max)
    {

        if (current_step < 20.0)
        {
            current_step += 1;
        }
        else if (current_step < 100.0)
        {
            current_step += 10.0;
        }
        else if (current_step < 1000.0)
        {
            current_step += 50.0;
        }
        else
        {
            current_step += 100.0;
        }
        if (current_step < max)
        {
            vec.push_back(current_step);
        }
    }

    if (vec.back() != max)
    {
        vec.push_back(max);
    }
    return vec;
}

tcam_image_size tcam::calculate_auto_center(const tcam_image_size& sensor,
                                            const tcam_image_size& step,
                                            const tcam_image_size& image,
                                            const image_scaling& scale)
{
    tcam_image_size ret = {};

    if (image.width > sensor.width || image.height > sensor.height)
    {
        return ret;
    }



    ret.width = (sensor.width / 2) - (image.width * (scale.binning_h * scale.skipping_h) / 2);
    ret.height = (sensor.height / 2) - (image.height * (scale.binning_v * scale.skipping_v)  / 2);

    ret.width -= ret.width % step.width;
    ret.height -= ret.height % step.height;

    if (!scale.legal_resolution(sensor, ret))
    {
        SPDLOG_ERROR("Unable to calculate auto center. This should not happen!");
        return {0, 0};
    }

    return ret;
}


bool tcam::compare_double(double val1, double val2)
{
    return std::fabs(val1 - val2) < std::numeric_limits<double>::epsilon();
}


bool tcam::in_range(const tcam_image_size& minimum,
                    const tcam_image_size& maximum,
                    const tcam_image_size& value)
{
    if (minimum.width > value.width || maximum.width < value.width)
    {
        return false;
    }
    if (minimum.height > value.height || maximum.height < value.height)
    {
        return false;
    }
    return true;
}


unsigned int tcam::get_pid_from_lockfile(const std::string& filename)
{
    std::ifstream f(filename);
    unsigned int ret = 0;
    if (f.is_open())
    {
        std::string line;
        getline(f, line);

        try
        {
            ret = std::stoi(line);
        }
        catch (const std::invalid_argument& e)
        {
            SPDLOG_ERROR("Could not convert line \"{}\" to valid pid.", line.c_str());
        }
        f.close();
    }
    else
    {
        SPDLOG_INFO("Could not open file \"{}\"", filename.c_str());
    }

    return ret;
}


bool tcam::is_process_running(unsigned int pid)
{
    int ret = kill(pid, 0);

    bool is_running = false;
    if (ret < 0)
    {
        if (errno == EPERM)
        {
            is_running = true;
        }
    }
    else
    {
        is_running = true;
    }
    return is_running;
}


double tcam::map_value_ranges(double input_start,
                              double input_end,
                              double output_start,
                              double output_end,
                              double value)
{
    return (value - input_start) * (output_end - output_start) / (input_end - input_start)
           - output_start;
}


bool tcam::is_environment_variable_set (const std::string& name)
{
    char* value = getenv(name.c_str());

    if (!value)
    {
        return false;
    }
    return true;
}


std::string tcam::get_environment_variable(const std::string& name, const std::string& backup)
{
    char* value = getenv(name.c_str());

    if (!value)
    {
        return backup;
    }

    return value;
}


std::optional<int> tcam::get_environment_variable_int(const std::string& name)
{
    char* value = getenv(name.c_str());

    if (!value)
    {
        return {};
    }

    try
    {
        return std::stoi(value);
    }
    catch (const std::exception& e)
    {
        SPDLOG_WARN("Failed to parse environment variable '{}' contents={}.", name, value);
    }
    return {};
}

int tcam::set_thread_name(const char* name, pthread_t thrd /*= pthread_self()*/)
{
    return pthread_setname_np(thrd, name);
}

int set_thread_name(const std::string& name, pthread_t thrd /*= pthread_self()*/)
{
    assert(name.size() <= 16);
    return set_thread_name(name.c_str(), thrd);
}
