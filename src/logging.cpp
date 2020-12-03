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

static const char* loglevel2string (const enum TCAM_LOG_LEVEL level)
{
    switch (level)
    {
        case TCAM_LOG_OFF:
            return "OFF";
        case TCAM_LOG_TRACE:
            return "TRACE";
        case TCAM_LOG_DEBUG:
            return "DEBUG";
        case TCAM_LOG_INFO:
            return "INFO";
        case TCAM_LOG_WARNING:
            return "WARNING";
        case TCAM_LOG_ERROR:
            return "ERROR";
        default:
            return NULL;
    }
}


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
    callback(nullptr),
    logfile(nullptr)
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
        std::cerr << "my err handler: " << msg << std::endl;
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
    target = STDIO;
    log_file = "/tmp/tis.log";
}


void Logger::log (const char* module __attribute__((unused)),
                  enum TCAM_LOG_LEVEL _level,
                  const char* function,
                  int line,
                  const char* message,
                  va_list args)
{
    // if (_level < level)
    // {
    //     return;
    // }

    // local copy of va_list
    // required because vsnprintf calls va_arg()
    // thus making reusage in the 2. vsnprintf call impossible
    va_list tmp_args;
    va_copy(tmp_args, args);

    size_t size = vsnprintf(NULL, 0, message, tmp_args) + 1;
    char *msg = new char[size];

    va_end(tmp_args);

    vsnprintf(msg, size, message, args);

    // get current time
    auto now = std::chrono::system_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm = *std::localtime(&now_c);

    const size_t t_size = 80;
    std::vector<char> t0(t_size);
    std::vector<char> t(t_size);

    strftime(t0.data(), t_size -1, "%Y.%m.%dT%H:%M:%S:%%03u", &now_tm);
    snprintf(t.data(), t_size -1, t0.data(),  ms);

#pragma GCC diagnostic ignored "-Wformat-truncation"

    size_t buffer_size = snprintf(nullptr,
                                  0,
                                  "%-30s <%s> %s:%d: %s\n",
                                  /* ctime(&timer), */
                                  t.data(),
                                  loglevel2string(_level),
                                  function,
                                  line,
                                  msg) + 1;

#pragma GCC diagnostic pop

    /* write complete message */
    char *buffer = new char[buffer_size];

    sprintf(buffer,
            "%-30s <%s> %s:%d: %s\n",
            /* ctime(&timer), */
            t.data(),
            loglevel2string(_level),
            function,
            line,
            msg);

    switch (target)
    {
        case STDIO:
        {
            log_to_stdout(buffer);
            if (callback)
            {
                callback(cb_user_data, _level, function, line, message, args);
            }
            break;
        }
        case LOGFILE:
        {
            log_to_file(buffer);
            break;
        }
        case USER_DEFINED:
        {
            //logger.callback(_level, file, line, message, args);
            break;
        }
        default:
            break;
    }
    delete [] buffer;
    delete [] msg;
}


void Logger::log_to_stdout (const char* message)
{
    fprintf(stdout, "%s", message);
    fflush(stdout);
}


void Logger::log_to_file (const char* message __attribute__((unused)))
{}


void Logger::set_log_level (enum TCAM_LOG_LEVEL l)
{
    //level = l;
}


enum TCAM_LOG_LEVEL Logger::get_log_level () const
{
    // return level;
}


void Logger::set_target (enum TCAM_LOG_TARGET t)
{
    target = t;
}


enum TCAM_LOG_TARGET Logger::get_target () const
{
    return target;
}


void Logger::set_log_file (const std::string& filename)
{
    log_file = filename;
}


std::string Logger::get_log_file () const
{
    return log_file;
}


void Logger::set_external_callback (logging_callback c,
                                    void* user_data)
{
    callback = c;
    cb_user_data = user_data;
}


void Logger::delete_external_callback ()
{
    callback = nullptr;
}


void Logger::open_logfile ()
{
    if (!log_file.empty())
        logfile = fopen(log_file.c_str(), "a+");
}


void Logger::close_logfile ()
{
    if (logfile != NULL)
    {
        fclose(logfile);
        logfile = NULL;
    }
}


Logger& Logger::getInstance ()
{
    static Logger instance;

    return instance;
}



void tcam_set_logging_level (enum TCAM_LOG_LEVEL level)
{
    Logger::getInstance().set_log_level(level);
}


enum TCAM_LOG_LEVEL tcam_get_logging_level ()
{
    return Logger::getInstance().get_log_level();
}


void tcam_logging_init(enum TCAM_LOG_TARGET target, enum TCAM_LOG_LEVEL level)
{
    tcam_set_logging_target(target);
    tcam_set_logging_level(level);
}





void tcam_set_logging_target (enum TCAM_LOG_TARGET target)
{
    Logger::getInstance().set_target(target);
}


void tcam_set_logging_file (const char* logfile_name)
{
    Logger::getInstance().set_log_file(logfile_name);
}


const char* tcam_get_logging_file ()
{
    return Logger::getInstance().get_log_file().c_str();
}


void tcam_logging (enum TCAM_LOG_LEVEL level, const char* file, int line, const char* message, ...)
{
    if (Logger::getInstance().get_log_level() > level ||
        Logger::getInstance().get_log_level() == TCAM_LOG_OFF)
    {
        return;
    }

    va_list args;
    va_start(args, message);

    Logger::getInstance().log("", level, file, line, message, args);

    va_end(args);
}


void tcam_logging (const char* module, enum TCAM_LOG_LEVEL level, const char* function, int line, const char* message, ...)
{
    if (Logger::getInstance().get_log_level() > level ||
        Logger::getInstance().get_log_level() == TCAM_LOG_OFF)
    {
        return;
    }

    va_list args;
    va_start(args, message);

    Logger::getInstance().log(module, level, function, line, message, args);

    va_end(args);
}
