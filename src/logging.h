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

#ifndef TCAM_LOGGING_H
#define TCAM_LOGGING_H

#include "compiler_defines.h"

#include <stdarg.h> /* va_args */
#include <string>


#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "spdlog/spdlog.h"


enum TCAM_LOG_LEVEL
{
    TCAM_LOG_OFF = 0,
    TCAM_LOG_TRACE = 1,
    TCAM_LOG_DEBUG = 2,
    TCAM_LOG_INFO = 3,
    TCAM_LOG_WARNING = 4,
    TCAM_LOG_ERROR = 5,
};

typedef void (
    *logging_callback)(void* user_data, enum TCAM_LOG_LEVEL, const char*, int, const char*, ...);


class Logger
{

public:
    static Logger& getInstance();

    void set_external_callback(logging_callback cb_function, void* user_data);
    void delete_external_callback();

private:
    Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Logger(Logger&&) = delete;
    Logger operator=(Logger&&) = delete;
    void load_default_settings();

    enum spdlog::level::level_enum level;
    logging_callback callback;
    void* cb_user_data;
};

#endif /* TCAM_LOGGING_H */
