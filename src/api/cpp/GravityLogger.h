/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

#ifndef GRAV_LOGGER_23459
#define GRAV_LOGGER_23459
#include "GravityNode.h"
#include "GravitySemaphore.h"
#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <list>

//class GravityNode {};

namespace gravity
{

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
 * Interface for a log writer.
 */
class Logger
{
public:
    /** 
     * Called by the logger when the user logs at a level which the instance of this class is set to log at.  
     * \param level log level 
     * \param messagestr log message
     */
    GRAVITY_API virtual void Log(int level, const char* messagestr) = 0;

    /** Default Destructor */
    GRAVITY_API virtual ~Logger() {}
};

/**
 * Manages logging and stores information about the logging state.  
 */
class Log
{
public:
    /**
     * Log Levels
     */
    enum LogLevel
    {
        NONE = 0,                              ///< logging is off
        FATAL = 1,                             ///< only log fatal level messages
        CRITICAL = 1 << GRAVITY_LOG_CRITICAL,  ///< log critical level messages and above
        WARNING = 1 << GRAVITY_LOG_WARNING,    ///< log warning level messages and above
        MESSAGE = 1 << GRAVITY_LOG_MESSAGE,    ///< log message level  messages and above
        DEBUG = 1 << GRAVITY_LOG_DEBUG,        ///< log debug level messages and above
        TRACE = 1 << GRAVITY_LOG_TRACE         ///< log trace level messages and above
    };
    /**
     * @name Helper functions
     * @{
     *  Functions to switch between LogLevel and const char*.
     */
    /** Returns a string representing the log level or NULL if it is an invalid log level */
    GRAVITY_API static const char* LogLevelToString(LogLevel level);
    /** Parses the string into one of the log levels.  Use "FATAL", CRITICAL", "WARNING",
     * "MESSAGE", "DEBUG" and "TRACE" for the strings.  Defaults to LogLevel::NONE on error.
     * This function is not case sensitive.
     */
    GRAVITY_API static LogLevel LogStringToLevel(const char* string);
    /** @} */  //Helper functions

    /**
     * Initialize a Logger to log to a file.  
     * Note that you do not have to create the Logger - this function does it for you.
     * May be called multiple times.
     * \param filename                 Existing directory to place log file in. Use "" to place in current directory.  Use "/dev/null" and set local_log_level to Log::NONE to turn off local logging.
     * \param comp_id                  ID of the component being logged. Will be the name of the log file.
     * \param local_log_level          The initial local logging level.
     * \param close_file_after_write   Boolean that indicates whether the log file should be kept open between writes.  Defaults to false.
     */
    GRAVITY_API static void initAndAddFileLogger(const char* directory, const char* comp_id, LogLevel local_log_level,
                                                 bool close_file_after_write = false);

    /**
     * Initialize a Logger to log to the console.
     * Note that you do not have to create the Logger - this function does it for you.
     * \param comp_id          ID of the component being logged.
     * \param local_log_level  The initial local logging level.
     */
    GRAVITY_API static void initAndAddConsoleLogger(const char* comp_id, LogLevel local_log_level);

    /**
     * Initialize a Logger.  
     * May be called along with other init functions.
     * After calling this function the Logger is now owned by this class (for now).
     * \param logger           A valid Logger to initialize.
     * \param log_level        The logging level to initialize this logger with.
     */
    GRAVITY_API static void initAndAddLogger(Logger* logger, LogLevel log_level);

    /**
     * Close all open Loggers.
     * Deallocates memory used for Loggers.
     */
    GRAVITY_API static void CloseLoggers();

    /**
     * @name Logging functions
     * @{
     *  Create log messages at specific levels.
     *  \param message  The log message format string.  Use printf style.
     *  \param ...      Addition printf style parameters
     */
    GRAVITY_API static void fatal(const char* message, ...);
    GRAVITY_API static void critical(const char* message, ...);
    GRAVITY_API static void warning(const char* message, ...);
    GRAVITY_API static void message(const char* message, ...);
    GRAVITY_API static void debug(const char* message, ...);
    GRAVITY_API static void trace(const char* message, ...);

    /** @} */  //Logging Functions

    /**
     * Removes the specified Logger.
     * Deallocates memory used for Logger.
     */
    GRAVITY_API static void RemoveLogger(Logger* logger);

    /**
     * Return the number of open loggers.
     */
    GRAVITY_API static int NumberOfLoggers();

private:
    /**
     * Calls the Logger::Log function for each initialized Logger 
     */
    static void vLog(int level, const char* format, va_list args);

    /**
     * Returns the LogLevel enum as an int.
     */
    static int LevelToInt(LogLevel level);

    static int32_t detectPercentN(const char* format);

    /**
     * List of all initialized Loggers.
     */
    static std::list<std::pair<Logger*, int> > loggers;
    static Semaphore lock;
};

}  // namespace gravity
#endif
