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

namespace gravity{

#define GRAVITY_LOG_FATAL 0
#define GRAVITY_LOG_CRITICAL 1
#define GRAVITY_LOG_WARNING 2
#define GRAVITY_LOG_MESSAGE 3
#define GRAVITY_LOG_DEBUG 4
#define GRAVITY_LOG_TRACE 5

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
	    static const char* LogLevelToString(LogLevel level);
	    static LogLevel LogStringToLevel(const char* string);
	
	    static void initAndAddFileLogger(const char* log_dir, const char* comp_id, LogLevel local_log_level);
	
	    static void initAndAddConsoleLogger(const char* comp_id, LogLevel local_log_level);
	
	    static void initAndAddLogger(gravity::Logger* logger, LogLevel log_level);
	
	    static void CloseLoggers();
	
	    static void fatal(const char* message, ...);
	    static void critical(const char* message, ...);
	    static void warning(const char* message, ...);
	    static void message(const char* message, ...);
	    static void debug(const char* message, ...);
	    static void trace(const char* message, ...);
	
	};
};