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

#include "GravityNode.h"
#include "GravityLogger.h"
#include "protobuf/GravityLogMessagePB.pb.h"
#include <stdarg.h>
#include <time.h>
#ifndef WIN32
#include <sys/time.h>
#endif
#include <string.h>
#include <stdio.h>
#include <iostream>
#include "Utility.h"

using namespace std;
using namespace gravity;

////////////////////////////////////
// Logger Classes
//

/**
 * Logs to a file
 */
class FileLogger : public Logger {
public:
    FileLogger(const string& log_dir, const string& comp_id, bool close_file);
    /*
     * NOTE: no need to filter on level!  This has already been done.
     */
    virtual void Log(int level, const char* messagestr);
    virtual ~FileLogger();
protected:
    FileLogger(const string& comp_id) {component_id = comp_id; close_file_after_write = false;}  // always keep open here
    string filename;
    FILE* log_file;
    string component_id;
    bool close_file_after_write;
};

FileLogger::FileLogger(const string& log_dir, const string& comp_id, bool close_file)
{
    component_id = comp_id;
    close_file_after_write = close_file;
#ifdef WIN32
    string sep_str = "\\";
#else
    string sep_str = "/";
#endif
    if (log_dir.length() == 0 || log_dir.compare (log_dir.length() - sep_str.length(), sep_str.length(), sep_str) == 0)
        filename = log_dir + component_id + ".log";
    else
        filename = log_dir + sep_str + component_id + ".log";

    log_file = fopen(filename.c_str(), "a");
    if(log_file == NULL)
    {
        cerr << "[Log::init] Could not open log file: " << filename << endl;
        log_file = fopen("Gravity.log", "a");
        if(log_file == NULL)
        {
            log_file = fopen("/dev/null", "a");
            cerr << "[Log::init] Could not open log file: Gravity.log" << endl;
        }
    }
    if (close_file_after_write)
    {
        fclose(log_file);
    }
}

void FileLogger::Log(int level, const char* messagestr)
{
    //Format the Logs nicely.
    char timestr[100];
    struct tm * timeinfo;
    time_t rawtime;
    string format = "%m/%d/%y %H:%M:%S";

#ifdef WIN32
    time ( &rawtime );
    timeinfo = localtime( &rawtime );
    strftime(timestr, 100, format.c_str(), timeinfo);
#else
    struct timeval  tv;
    gettimeofday(&tv, NULL);
    rawtime = tv.tv_sec;
    timeinfo = localtime( &rawtime );
    format = format + ".%%06u";
    char buffer[100];
    strftime(buffer, 100, format.c_str(), timeinfo);
    snprintf(timestr, sizeof timestr, buffer, tv.tv_usec);
#endif

    if (close_file_after_write)
    {
        log_file = fopen(filename.c_str(), "a");
        if(log_file == NULL)
        {
            cerr << "[Log::init] Could not open log file: " << filename << endl;
            log_file = fopen("Gravity.log", "a");
            if(log_file == NULL)
            {
                log_file = fopen("/dev/null", "a");
                cerr << "[Log::init] Could not open log file: Gravity.log" << endl;
                return;
            }
        }
    }
    fprintf(log_file, "[%s %s-%s] ", timestr, component_id.c_str(), Log::LogLevelToString((Log::LogLevel)level));

    fputs(messagestr, log_file);
    fputs("\n", log_file);

    fflush(log_file); //I'm not sure when or how often this should be called.

    if (close_file_after_write)
    {
        fclose(log_file);
    }
}

FileLogger::~FileLogger()
{
    fclose(log_file);
}

void Log::initAndAddFileLogger(const char* log_dir, const char* comp_id, LogLevel local_log_level, bool close_file_after_write)
{
    Log::initAndAddLogger(new FileLogger(log_dir, comp_id, close_file_after_write), local_log_level);
}

/**
 * Logs to the console.
 */
class ConsoleLogger : public FileLogger
{
public:
    ConsoleLogger(const string& comp_id);
};

ConsoleLogger::ConsoleLogger(const string& comp_id) : FileLogger(comp_id)
{
    log_file = stdout; //fdopen(dup(STDOUT_FILENO), "w"); //Duplicate the file handle so we can close it after we're done with it.
    //I hope this is the right way to do this.
}

void Log::initAndAddConsoleLogger(const char* comp_id, LogLevel local_log_level)
{
    Log::initAndAddLogger(new ConsoleLogger(comp_id), local_log_level);
}

////////////////////////////////////////////////////////////////
// Main Log Functions
//

//Initialize
std::list< std::pair<Logger*, int> > Log::loggers;
Semaphore Log::lock;

void Log::initAndAddLogger(Logger* logger, LogLevel log_level)
{
    lock.Lock();

    //safeguard against adding duplicate logger
    for (auto i = loggers.begin(); i != loggers.end(); ++i)
    {
      if (i->first == logger)
      {
        lock.Unlock();
        return;
      }
    }

    loggers.push_back(make_pair(logger, Log::LevelToInt(log_level)));
    lock.Unlock();
}

void Log::RemoveLogger(Logger* logger)
{
  lock.Lock();
  std::list< std::pair<Logger*, int> >::iterator i = loggers.begin();
	while(i != loggers.end())
	{
		if(i->first == logger)
		{
      delete i->first;
			i = loggers.erase(i);
		}
		else
		{
		    i++;
		}
	}
  lock.Unlock();
}

const char* Log::LogLevelToString(LogLevel level)
{
    if(level == Log::FATAL)
        return "FATAL";
    else if(level == Log::CRITICAL)
        return "CRITICAL";
    else if(level == Log::WARNING)
        return "WARNING";
    else if(level == Log::MESSAGE)
        return "MESSAGE";
    else if(level == Log::DEBUG)
        return "DEBUG";
    else if(level == Log::TRACE)
        return "TRACE";
    return ""; //Get Rid of Warning.
}

Log::LogLevel Log::LogStringToLevel(const char* level)
{
    char* llevel = strdup(level);
    StringToLowerCase(llevel, strlen(llevel));
    Log::LogLevel ret = Log::NONE;
    if(strcmp(llevel, "fatal") == 0)
        ret = Log::FATAL;
    else if(strcmp(llevel, "critical") == 0)
        ret = Log::CRITICAL;
    else if(strcmp(llevel, "warning") == 0)
        ret = Log::WARNING;
    else if(strcmp(llevel, "message") == 0)
        ret = Log::MESSAGE;
    else if(strcmp(llevel, "debug") == 0)
        ret = Log::DEBUG;
    else if(strcmp(llevel, "trace") == 0)
        ret = Log::TRACE;
    free(llevel);
    return ret;
}

int32_t Log::detectPercentN(const char* format)
{

    const char subspecs[] = {// flags
                             '-', '+', '0', ' ', '#',
                             // width/precision
                             '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '*',
                             // modifiers
                             'h', 'l', 'L', 'z', 'j', 't' };
    size_t pos = 0;
    std::string checkStr(format);
    while (pos < checkStr.length())
    {
        pos = checkStr.find_first_of('%', pos);
        if (pos == checkStr.npos)
        {
            break;
        }
        size_t percentPos = pos++;
        pos = checkStr.find_first_not_of(subspecs, pos);
        if (checkStr[pos] == 'n')
        {
            return percentPos;
        }
    }
    return -1;
}

void Log::vLog(int level, const char* format, va_list args)
{
    lock.Lock();
    std::list< std::pair<Logger*, int> >::const_iterator i = loggers.begin();
    std::list< std::pair<Logger*, int> >::const_iterator l_end = loggers.end();
    if(i != l_end)
    {
        const uint32_t maxStrLen = 4096;
        char messageStr[maxStrLen];
        int percentNPos = detectPercentN(format);
        if (percentNPos >= 0)
        {
            const char* truncStr = " !!!!!!!!! '%n' DETECTED, MESSAGE TRUNCATED";
            uint32_t truncLen = percentNPos;
            if (truncLen > maxStrLen - strlen(truncStr) - 1)
            {
                truncLen = maxStrLen - strlen(truncStr) - 1;
            }
            char truncFormat[maxStrLen];
            // need to create a copy of the format string that ends before the %n
            // even if the the len provided to vsnprintf means it won't reach that point.
            strncpy(truncFormat, format, truncLen);
            truncFormat[truncLen] = (char)NULL;
            vsnprintf(messageStr, truncLen, truncFormat, args);
            // vsnprintf does not null terminate on all platforms
            messageStr[truncLen] = (char)NULL;
            strcat(messageStr, truncStr);
        }
        else
        {
            vsnprintf(messageStr, maxStrLen, format, args);
			// make sure null terminated
			messageStr[maxStrLen-1] = (char)NULL;
        }

        do
        {
            if(i->second & level)
                i->first->Log(level, messageStr);
            i++;
        } while(i != l_end);
    }
    lock.Unlock();
}

int Log::LevelToInt(LogLevel level)
{
    int int_level;
    switch(level)
    {
        case FATAL:
            int_level = FATAL;
            break;
        case CRITICAL:
            int_level = FATAL | CRITICAL;
            break;
        case WARNING:
            int_level = FATAL | CRITICAL | WARNING;
            break;
        case MESSAGE:
            int_level = FATAL | CRITICAL | WARNING | MESSAGE;
            break;
        case DEBUG:
            int_level = FATAL | CRITICAL | WARNING | MESSAGE | DEBUG;
            break;
        case TRACE:
            int_level = FATAL | CRITICAL | WARNING | MESSAGE | DEBUG | TRACE;
            break;
        case NONE:
        	int_level = 0;
        	break;
        default:
        	int_level = FATAL | CRITICAL | WARNING | MESSAGE | DEBUG | TRACE;
        	break;
    }

    return int_level;
}


void Log::CloseLoggers()
{
    lock.Lock();
    std::list< std::pair<Logger*, int> >::iterator i = loggers.begin();
    
    //Remove all Loggers
    while(i != loggers.end())  
    {
      delete i->first;
			i = loggers.erase(i);
    }
    lock.Unlock();
}

int Log::NumberOfLoggers()
{
  lock.Lock();
  int size = loggers.size();
  lock.Unlock(); 
  return size;
}

//Using functions instead of macros so we can use namespaces.
#if COMPILE_GRAVITY_LOGGING_LEVEL >= GRAVITY_LOG_FATAL
void Log::fatal(const char* message, ...) {
    va_list args;
    va_start ( args, message );
    Log::vLog(FATAL, message, args);
    va_end ( args );
}
#else
void Log::fatal(const char* message, ...) { }
#endif

#if COMPILE_GRAVITY_LOGGING_LEVEL >= GRAVITY_LOG_CRITICAL
void Log::critical(const char* message, ...) {
    va_list args1;
    va_start ( args1, message );
    Log::vLog(CRITICAL, message, args1);
    va_end ( args1 );
}
#else
void Log::critical(const char* message, ...) { }
#endif

#if COMPILE_GRAVITY_LOGGING_LEVEL >= GRAVITY_LOG_WARNING
void Log::warning(const char* message, ...) {
    va_list args;
    va_start ( args, message );
    Log::vLog(WARNING, message, args);
    va_end ( args );
}
#else
void Log::warning(const char* message, ...) { }
#endif

#if COMPILE_GRAVITY_LOGGING_LEVEL >= GRAVITY_LOG_MESSAGE
void Log::message(const char* message, ...) {
    va_list args;
    va_start ( args, message );
    Log::vLog(MESSAGE, message, args);
    va_end ( args );
}
#else
void Log::message(const char* message, ...) { }
#endif


#if COMPILE_GRAVITY_LOGGING_LEVEL >= GRAVITY_LOG_DEBUG
void Log::debug(const char* message, ...) {
    va_list args;
    va_start ( args, message );
    Log::vLog(DEBUG, message, args);
    va_end ( args );
}
#else
void Log::debug(const char* message, ...) { }
#endif

#if COMPILE_GRAVITY_LOGGING_LEVEL >= GRAVITY_LOG_TRACE
void Log::trace(const char* message, ...) {
    va_list args;
    va_start ( args, message );
    Log::vLog(TRACE, message, args);
    va_end ( args );
}
#else
void Log::trace(const char* message, ...) { }
#endif
