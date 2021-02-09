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

#include "logging.h"
#include "version.h"

#include <stdio.h>              /* printf, fopen */
#include <stdarg.h>             /* va_args */
#include <string.h>             /* memcpy */
#include <time.h>               /* time_t */
#include <vector>
#include <chrono>


static spdlog::level::level_enum string2loglevel (const char* level)
{

    if (strcmp("OFF", level) == 0)
    {
        return spdlog::level::off;
    }
    else if (strcmp("TRACE", level) == 0)
    {
        return spdlog::level::trace;
    }
    else if (strcmp("DEBUG", level) == 0)
    {
        return spdlog::level::debug;
    }
    else if (strcmp("INFO", level) == 0)
    {
        return spdlog::level::info;
    }
    else if (strcmp("WARNING", level) == 0)
    {
        return spdlog::level::warn;
    }
    else if (strcmp("ERROR", level) == 0)
    {
        return spdlog::level::err;
    }
    else
        return spdlog::level::err;
}


Logger::Logger ():
    callback(nullptr)

{

#ifndef TCAM_LOG_ENV_NAME
#define TCAM_LOG_ENV_NAME "TCAM_LOG"
#endif



    static const char* env_name = TCAM_LOG_ENV_NAME;

    load_default_settings();
    char* log_def = getenv(env_name);
    if (log_def != nullptr)
    {
        level = string2loglevel(log_def);
    }

    spdlog::set_pattern("[%Y%m%dT%T] [%^%-7l%$] %s:%#: %v");
    // spdlog::set_level(spdlog::level::debug);
    spdlog::set_level(level);

    spdlog::set_error_handler([](const std::string& msg)
    {
        SPDLOG_ERROR("Error while handling logging message: {}", msg);
    });

    char b[1024];
    sprintf(b,
            "\nThe following library versions are used:\n\tTcam:\t%s\n\tAravis:\t%s\n\tModules:\t%s",
            get_version(),
            get_aravis_version(),
            get_enabled_modules());

    SPDLOG_INFO(b);

}

void Logger::load_default_settings ()
{
    level = spdlog::level::err;

}




Logger& Logger::getInstance ()
{
    static Logger instance;

    return instance;
}
