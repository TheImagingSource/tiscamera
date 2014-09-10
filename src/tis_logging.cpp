
#include "tis_logging.h"

#include <stdio.h>              /* printf, fopen */
#include <stdarg.h>             /* va_args */
#include <string.h>             /* memcpy */
#include <time.h>               /* time_t */

struct tis_logging_info
{
    enum TIS_LOG_LEVEL  level;
    enum TIS_LOG_TARGET target;

    char  logfile_name[256];
    FILE* logfile;

    logging_callback callback;
};

/* static struct tis_logging_info logger = { ERROR, STDIO, "/tmp/tis.log", NULL, NULL}; */
static struct tis_logging_info logger = { TIS_LOG_DEBUG, STDIO, "/tmp/tis.log", NULL, NULL};


void tis_set_logging_target (enum TIS_LOG_TARGET target)
{
    logger.target = target;
}


void tis_set_logging_file (const char* logfile_name)
{
    memcpy(logger.logfile_name, logfile_name, sizeof(logger.logfile_name));
}


const char* tis_get_logging_file ()
{
    return logger.logfile_name;
}


static const char* loglevel2string (const enum TIS_LOG_LEVEL level)
{
    switch (level)
    {
        case TIS_LOG_OFF:
            return "OFF";
        case TIS_LOG_DEBUG:
            return "DEBUG";
        case TIS_LOG_INFO:
            return "INFO";
        case TIS_LOG_WARNING:
            return "WARNING";
        case TIS_LOG_ERROR:
            return "ERROR";
        default:
            return NULL;
    }
}

void tis_set_logging_level (enum TIS_LOG_LEVEL level)
{
    logger.level = level;
}


enum TIS_LOG_LEVEL tis_get_logging_level ()
{
    return logger.level;
}


void tis_logging_init(enum TIS_LOG_TARGET target, enum TIS_LOG_LEVEL level)
{
    tis_set_logging_target(target);
    tis_set_logging_level(level);
}



static void open_logfile ()
{
    logger.logfile = fopen(logger.logfile_name, "a+");
}


static void close_logfile ()
{
    if (logger.logfile != NULL)
    {
        fclose(logger.logfile);
        logger.logfile = NULL;
    }
}


static void log_logfile (const char* message)
{
    if (strcmp(logger.logfile_name, "") == 0)
    {
        return;
    }

    if (logger.logfile == NULL)
    {
        open_logfile();
        if (logger.logfile == NULL)
        {
            return;
        }
    }

    fwrite(message, sizeof(char), strlen(message), logger.logfile);
    
    close_logfile();
}


static void log_stdio (const char* message)
{
    fprintf(stdout, "%s", message);
    fflush(stdout);
}


void tis_logging (enum TIS_LOG_LEVEL level, const char* file, int line, const char* message, ...)
{
    if (level < logger.level)
    {
        return;
    }
    va_list args;
    va_start(args, message);

    char msg[1024];
    char buffer [2056];

    /* fill user defined message */
    vsprintf(msg, message, args);

    /* time_t timer = {0}; */
    /* time(&timer); */

/* char t [64]; */

    clock_t t;
    t = clock();
    /* write complete message */
    sprintf(buffer,
            "%-10ld <%s> %s:%d: %s\n",
            /* ctime(&timer), */
            t,
            loglevel2string(level),
            file,
            line,
            msg);

/* printf("%s", buffer); */
    
    switch (logger.target)
    {
        case STDIO:
            log_stdio(buffer);
            break;
        case LOGFILE:
            log_logfile(buffer);
            break;
        case USER_DEFINED:
            logger.callback(level, file, line, message, args);
            break;
        default:
            break;
    }
    va_end(args);
}

/* #define mylog(message,...) logging(__FILE__ , __LINE__, message, ## __VA_ARGS__) */

