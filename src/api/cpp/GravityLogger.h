#ifndef GRAV_LOGGER_23459
#define GRAV_LOGGER_23459
#include "GravityNode.h"
#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <list>

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

class Logger {
public:
    virtual void Log(int level, const char* messagestr) = 0;
    virtual ~Logger() {}
};

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
    /*
     * Helper function.
     */
    static const char* LogLevelToString(LogLevel level);
    /*
     * Helper function.
     */
    static LogLevel LogStringToLevel(const char* string);

    /*
     * You must call this function to initialize the Logger with File logging to the file specified.  May be called multiple times.
     * \param filename         Local logging filename.  Use "/dev/null" and set local_log_level to Log::NONE to turn off local logging.
     * \param log_local_level  The initial local logging level.
     */
    static void initAndAddFileLogger(const char* filename, LogLevel local_log_level);
    /*
     * You must call this function to initialize the Logger with Gravity network logging.  May be called in addition to initAndAddFileLogger().
     * \param gravity_node     The GravityNode with which to connect to the remote log recorder machine.  Can be NULL for logging only to a file.
     * \param log_net_level    The initial network logging leve.
     */
    static void initAndAddGravityLogger(GravityNode *gravity_node, unsigned short port, LogLevel net_log_level);
    /*
     * You must call this function to initialize the Logger with a generic Logger.  May be called along with other init functions.
     * Called by initAndAddFileLogger() and initAndAddGravityLogger().
     * \param gravity_node     The GravityNode with which to connect to the remote log recorder machine.  Can be NULL for logging only to a file.
     * \param log_level        The logging level to initialize this logger with.
     */
    static void initAndAddLogger(Logger* logger, LogLevel log_level);


    /*
     * Logging functions:
     *  Use these to log messages.
     *  \param message  The log message format string.  Use printf style.
     *  \param ...      Addition printf style parameters
     */
    static void fatal(const char* message, ...);
    static void critical(const char* message, ...);
    static void warning(const char* message, ...);
    static void message(const char* message, ...);
    static void debug(const char* message, ...);
    static void trace(const char* message, ...);

    static void AddLogger(Logger* logger);
    //static void RemoveLogger(Logger* logger);
private:
    static void vLog(int level, const char* format, va_list args);
    int LevelToInt(LogLevel level);

    static std::list< std::pair<Logger*, int> > loggers;
};

} //Namespace
#endif
