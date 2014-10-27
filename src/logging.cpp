
#include "logging.h"

#include <stdio.h>              /* printf, fopen */
#include <stdarg.h>             /* va_args */
#include <string.h>             /* memcpy */
#include <time.h>               /* time_t */


static const char* loglevel2string (const enum TCAM_LOG_LEVEL level)
{
    switch (level)
    {
        case TCAM_LOG_OFF:
            return "OFF";
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


Logger::Logger ():
    callback(nullptr),
    logfile(nullptr)
{
    // TODO: make environment variable name configurable
    char* log_def = getenv("TCAM_LOG");
    if (log_def == nullptr)
    {
        load_default_settings();
    }
}

void Logger::load_default_settings ()
{
    level = TCAM_LOG_DEBUG;
    target = STDIO;
    log_file = "tmp/tis.log";
}


void Logger::log (const char* module,
                  enum TCAM_LOG_LEVEL level,
                  const char* function,
                  int line,
                  const char* message,
                  va_list args)
{
    if (level < this->level)
    {
        return;
    }

    char msg[1024];
    char buffer [2056];

    /* fill user defined message */
    vsprintf(msg, message, args);

    clock_t t;
    t = clock();
    /* write complete message */
    sprintf(buffer,
            "%-10ld <%s> %s:%d: %s\n",
            /* ctime(&timer), */
            t,
            loglevel2string(level),
            function,
            line,
            msg);

    switch (target)
    {
        case STDIO:
            log_to_stdout(buffer);
            break;
        case LOGFILE:
            log_to_file(buffer);
            break;
        case USER_DEFINED:
            //logger.callback(level, file, line, message, args);
            break;
        default:
            break;
    }
}


void Logger::log_to_stdout (const char* message)
{
    fprintf(stdout, "%s", message);
    fflush(stdout);
}


void Logger::log_to_file (const char* message)
{}


void Logger::set_log_level (enum TCAM_LOG_LEVEL l)
{
    level = l;
}


enum TCAM_LOG_LEVEL Logger::get_log_level () const
{
    return level;
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


void Logger::set_external_callback (logging_callback c)
{
    callback = c;
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
