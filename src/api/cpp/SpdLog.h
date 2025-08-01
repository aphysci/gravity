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

#ifndef SPD_LOG_H_
#define SPD_LOG_H_

#include "GravityNode.h"

namespace gravity
{

#define SPD_LOG_OFF 6
#define SPD_LOG_CRITICAL 5
#define SPD_LOG_ERROR 4
#define SPD_LOG_WARNING 3
#define SPD_LOG_INFO 2
#define SPD_LOG_DEBUG 1
#define SPD_LOG_TRACE 0

#undef ERROR

/**
 * Manages logging and stores information about the logging state.  
 */
class GRAVITY_API SpdLog
{
public:
    /**
     * Log Levels
     */
    enum LogLevel
    {
        TRACE,
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL,
        OFF
    };

    /**
     * @name Logging functions
     * @{
     *  Create log messages at specific levels.
     *  \param message  The log message format string.  Use printf style.
     *  \param ...      Addition printf style parameters
     */
    static void critical(const char* message);
    static void error(const char* message);
    static void warn(const char* message);
    static void info(const char* message);
    static void debug(const char* message);
    static void trace(const char* message);
};

}  // namespace gravity
#endif
