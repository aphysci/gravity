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

/**
 * Interface for various logging functionality.  Use this class to create a custom log writer.
 * Use the AddLogger() function to start logging with a derived version of this class.
 */
class Logger {
public:
    /** Called by the logger when the user logs at a level which the instance of this class is set to log at.  */
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
    /**
     * Helper functions
     * @{
     */
    /** Returns a string representing the log level or NULL if it is an invalid log level */
    GRAVITY_API static const char* LogLevelToString(LogLevel level);
    /** Parses the string into one of the log levels.  Use "FATAL", CRITICAL", "WARNING",
     * "MESSAGE", "DEBUG" and "TRACE" for the strings.  Defaults to LogLevel::NONE on error.
     * This function is not case sensitive.
     */
    GRAVITY_API static LogLevel LogStringToLevel(const char* string);
    /** @} */ //Helper functions

    /**
     * You must call this function to initialize the Logger with File logging to the file specified.  May be called multiple times.
     * \param filename         Local logging filename.  Use "/dev/null" and set local_log_level to Log::NONE to turn off local logging.
     * \param log_local_level  The initial local logging level.
     */
    GRAVITY_API static void initAndAddFileLogger(const char* filename, LogLevel local_log_level);

    /**
     * Adds a console Logger.
     * \param log_local_level  The initial local logging level.
     */
    GRAVITY_API static void initAndAddConsoleLogger(LogLevel local_log_level);

    /**
     * You must call this function to initialize the Logger with Gravity network logging.  May be called in addition to initAndAddFileLogger().
     * \param gravity_node     The GravityNode with which to connect to the remote log recorder machine.  Can be NULL for logging only to a file.
     * \param log_net_level    The initial network logging leve.
     */
    GRAVITY_API static void initAndAddGravityLogger(GravityNode *gravity_node, unsigned short port, LogLevel net_log_level);
    /**
     * You must call this function to initialize the Logger with a generic Logger.  May be called along with other init functions.
     * Called by initAndAddFileLogger() and initAndAddGravityLogger().
     * After calling this function the Logger is now owned by this class (for now).
     * \param gravity_node     The GravityNode with which to connect to the remote log recorder machine.  Can be NULL for logging only to a file.
     * \param log_level        The logging level to initialize this logger with.
     */
    GRAVITY_API static void initAndAddLogger(Logger* logger, LogLevel log_level);

    /**
     * Closes and deletes all open loggers.
     */
    GRAVITY_API static void CloseLoggers();

    /**
     * @name Logging functions
     * @{
     *  Use these functions to log messages.
     *  \param message  The log message format string.  Use printf style.
     *  \param ...      Addition printf style parameters
     */
    GRAVITY_API static void fatal(const char* message, ...);
    GRAVITY_API static void critical(const char* message, ...);
    GRAVITY_API static void warning(const char* message, ...);
    GRAVITY_API static void message(const char* message, ...);
    GRAVITY_API static void debug(const char* message, ...);
    GRAVITY_API static void trace(const char* message, ...);
    /** @} */ //Logging Functions

    /**
     * Adds a custom logging function.  Used by the init functions.
     */
    GRAVITY_API static void RemoveLogger(Logger* logger);
private:
    static void vLog(int level, const char* format, va_list args);
    static int LevelToInt(LogLevel level);

    static std::list< std::pair<Logger*, int> > loggers;
};

} //Namespace
#endif
