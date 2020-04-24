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



%feature("director") gravity::CPPGravityLogger;

%typemap(javaimports) gravity::Log %{
import com.aphysci.gravity.Logger;
%}

namespace gravity{

#define GRAVITY_LOG_FATAL 0
#define GRAVITY_LOG_CRITICAL 1
#define GRAVITY_LOG_WARNING 2
#define GRAVITY_LOG_MESSAGE 3
#define GRAVITY_LOG_DEBUG 4
#define GRAVITY_LOG_TRACE 5

class CPPGravityLogger
{
public:
	virtual ~CPPGravityLogger();
    virtual void Log(int level, const char* messagestr);
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

/**
 * For all of the log methods, only take a single string, but provide that string to the C++
 * call as the second arg with "%s" as the format string.  This prevents issues with any text 
 * in the log message that may be interpretted as a format character.
 */
%typemap(in) (const char* format, const char* message) {
    $1 = (char *) malloc(4*sizeof(char *));
    sprintf($1, "%%s");
    $2 = (char *)jenv->GetStringUTFChars($input, 0);
    if (!$2) return ;
}
%typemap(jtype) (const char* format, const char* message) "String"
%typemap(jstype) (const char* format, const char* message) "String"
%typemap(javain) (const char* format, const char* message) "$javainput"
%typemap(freearg) (const char* format, const char* message) {
    free((char *) $1);
    if ($2) jenv->ReleaseStringUTFChars($input, (const char *)$2);
}
    static void fatal(const char* format, const char* message);
    static void critical(const char* format, const char* message);
    static void warning(const char* format, const char* message);
    static void message(const char* format, const char* message);
    static void debug(const char* format, const char* message);
    static void trace(const char* format, const char* message);

};

};
