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

#include <stdarg.h>             /* va_args */
#include <string>

#include "compiler_defines.h"


enum TCAM_LOG_TARGET
{
    NONE         = 0,
    STDIO        = 1,
    LOGFILE      = 2,
    USER_DEFINED = 3,
};

/*
  @brief Define target for logging output
*/
void tcam_set_logging_target (enum TCAM_LOG_TARGET target);

void tcam_set_logging_file (const char* logfile);

const char* tcam_get_logging_file ();


enum TCAM_LOG_LEVEL
{
    TCAM_LOG_OFF     = 0,
    TCAM_LOG_DEBUG   = 1,
    TCAM_LOG_INFO    = 2,
    TCAM_LOG_WARNING = 3,
    TCAM_LOG_ERROR   = 4,
};

typedef void (*logging_callback) (enum TCAM_LOG_LEVEL, const char*, int, const char*, ...);


class Logger
{

public:

    static Logger& getInstance ();

    void log (const char* module,
              enum TCAM_LOG_LEVEL level,
              const char* function,
              int line,
              const char* message,
              va_list);

    void set_log_level (enum TCAM_LOG_LEVEL);
    enum TCAM_LOG_LEVEL get_log_level () const;

    void set_target (enum TCAM_LOG_TARGET);
    enum TCAM_LOG_TARGET get_target () const;

    void set_log_file (const std::string& filename);
    std::string get_log_file () const;

    void set_external_callback (logging_callback);
    void delete_external_callback ();

private:

    Logger ();

    Logger (const Logger&) = delete;
    Logger& operator= (const Logger&) = delete;

    Logger(Logger &&) = delete;
    Logger operator=(Logger &&) = delete;

    void load_default_settings ();

    void log_to_stdout (const char* message);
    void log_to_file (const char* message);

    void open_logfile ();
    void close_logfile ();

    TCAM_LOG_LEVEL level;
    std::string log_file;
    TCAM_LOG_TARGET target;
    logging_callback callback;
    FILE* logfile;
};

/*
  @brief Set the general log level. Everything lower will be discarded.
*/
void tcam_set_logging_level (enum TCAM_LOG_LEVEL level);


enum TCAM_LOG_LEVEL tcam_get_logging_level ();

/*
  @brief Convenience function; wraps definitions of log-level, target,
  into one function
*/
void tcam_logging_init(enum TCAM_LOG_TARGET target, enum TCAM_LOG_LEVEL level);


/*
  @brief logging function; follows printf syntax
*/
void tcam_logging (enum TCAM_LOG_LEVEL level, const char* function, int line, const char* message, ...);

void tcam_logging (const char* module,
                  enum TCAM_LOG_LEVEL level,
                  const char* function,
                  int line,
                  const char* message,
                  ...);

/*
  Convience wrapper macro
*/
#define tcam_log(level, message, ...) (tcam_logging(level, __FILE__ , __LINE__, message, ##__VA_ARGS__))

#define tcam__log(module, level, message, ...) (tcam_logging(module, level, __FILE__ , __LINE__, message, ##__VA_ARGS__))

#define tcam_error(message, ...) (tcam_log(TCAM_LOG_ERROR, message, ##__VA_ARGS__))
#define tcam_warning(message, ...) (tcam_log(TCAM_LOG_WARNING, message, ##__VA_ARGS__))
#define tcam_info(message, ...) (tcam_log(TCAM_LOG_INFO, message, ##__VA_ARGS__))
#define tcam_debug(message, ...) (tcam_log(TCAM_LOG_DEBUG, message, ##__VA_ARGS__))

#endif /* TCAM_LOGGING_H */
