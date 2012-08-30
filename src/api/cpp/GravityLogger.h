#ifndef GRAV_LOGGER_23459
#define GRAV_LOGGER_23459
#include "GravityNode.h"
#include <string>
#include <stdarg.h>
#include <stdio.h>

//class GravityNode {};

namespace gravity {

#define GRAVITY_LOG_FATAL 0
#define GRAVITY_LOG_CRITICAL 1
#define GRAVITY_LOG_WARNING 2
#define GRAVITY_LOG_MESSAGE 3
#define GRAVITY_LOG_DEBUG 4
#define GRAVITY_LOG_TRACE 5

#ifndef COMPILE_GRAVITY_LOGGING_LEVEL
#define COMPILE_GRAVITY_LOGGING_LEVEL GRAVITY_LOG_TRACE
#endif

class Log
{
public:
    enum LogLevel {
        NONE = 0,
        FATAL = 1,
        CRITICAL = 1 << GRAVITY_LOG_CRITICAL,
        WARNING = 1 << GRAVITY_LOG_WARNING,
        MESSAGE = 1 << GRAVITY_LOG_MESSAGE,
        DEBUG = 1 << GRAVITY_LOG_DEBUG,
        TRACE = 1 << GRAVITY_LOG_TRACE
    };

    static void init(GravityNode *gravity_node, const char* filename, unsigned short port, LogLevel local_log_level, LogLevel net_log_level);

    static void vLog(int level, const char* format, va_list args);

    static void fatal(const char* message, ...);
    static void critical(const char* message, ...);
    static void warning(const char* message, ...);
    static void message(const char* message, ...);
    static void debug(const char* message, ...);
    static void trace(const char* message, ...);

    //TODO: write these accessors
    static void setLocalLevel(LogLevel level);
    static LogLevel getLocalLevel();

    static void setNetworkLevel(LogLevel level);
    static LogLevel getNetworkLevel();

    static const char* LogLevelToString(LogLevel level);
    static LogLevel LogStringToLevel(const char* string);

private:
    //std::map<std::string, std::map<LogLevel, logfunc> > log_config;
    static FILE* log_file;
    static int current_local_levels;
    static int current_network_levels;
    static std::string log_dataProductID;
    static GravityNode* gravity_node;
};

} //Namespace
#endif
