#include "GravityNode.h"
#include "GravityLogger.h"
#include "protobuf/GravityLogMessagePB.pb.h"
#include <stdarg.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace gravity;

////////////////////////////////////
// Logger Classes
//

/*
 * Logs to a file
 */
class FileLogger : public Logger {
public:
    FileLogger(const char* filename);
    /*
     * NOTE: no need to filter on level!  This has already been done.
     */
    virtual void Log(int level, const char* messagestr);
    virtual ~FileLogger();
protected:
    FileLogger() { }
    FILE* log_file;
};

FileLogger::FileLogger(const char* filename)
{
    log_file = fopen(filename, "a");
    if(log_file == NULL)
    {
        log_file = fopen("/dev/null", "a");
        cerr << "[Log::init] Could not open log file: " << filename << endl;
    }
}

void FileLogger::Log(int level, const char* messagestr)
{
    //Format the Logs nicely.
    char timestr[100];
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime( &rawtime );

    strftime(timestr, 100, "%m/%d/%y %H:%M:%S", timeinfo);

    fprintf(log_file, "[%s %s] ", Log::LogLevelToString((Log::LogLevel)level), timestr);

    fputs(messagestr, log_file);
    fputs("\n", log_file);

    //fflush(log_file); //I'm not sure when or how often this should be called.
}

FileLogger::~FileLogger()
{
    fclose(log_file);
}

void Log::initAndAddFileLogger(const char* filename, LogLevel local_log_level)
{
    Log::initAndAddLogger(new FileLogger(filename), local_log_level);
}


class ConsoleLogger : public FileLogger
{
public:
    ConsoleLogger();
};

ConsoleLogger::ConsoleLogger()
{
    log_file = stdout; //fdopen(dup(STDOUT_FILENO), "w"); //Duplicate the file handle so we can close it after we're done with it.
    //I hope this is the right way to do this.
}

void Log::initAndAddConsoleLogger(LogLevel local_log_level)
{
    Log::initAndAddLogger(new ConsoleLogger(), local_log_level);
}

/*
 * Logs to a GravityLogRecorder on the Network.
 */
class GravityLogger : public Logger
{
public:
    GravityLogger(GravityNode* gn, unsigned short port);
    virtual void Log(int level, const char* messagestr);
    virtual ~GravityLogger();
private:
    GravityNode* gravity_node;
    static std::string log_dataProductID;
};

std::string GravityLogger::log_dataProductID = "GRAVITY_LOGGER";

GravityLogger::GravityLogger(GravityNode* gn, unsigned short port)
{
    gravity_node = gn;
    if(gravity_node->registerDataProduct(log_dataProductID, port, "tcp") != GravityReturnCodes::SUCCESS)
        cerr << "[Log::init] Could not register Logger" << endl;
}

void GravityLogger::Log(int level, const char* messagestr)
{
    GravityDataProduct dp(log_dataProductID);

    //TODO: how is this supposed to work???
    gravity::GravityLogMessagePB log_message;
    log_message.set_domain("None");
    log_message.set_level(Log::LogLevelToString((Log::LogLevel)level));
    log_message.set_message(messagestr);

    dp.setData(log_message);

    gravity_node->publish(dp);
}

GravityLogger::~GravityLogger()
{
}


void Log::initAndAddGravityLogger(GravityNode *gn, unsigned short port, LogLevel net_log_level)
{
    Log::initAndAddLogger(new GravityLogger(gn, port), net_log_level);
}

////////////////////////////////////////////////////////////////
// Main Log Functions
//

std::list< std::pair<Logger*, int> > Log::loggers; //Initialize

void Log::initAndAddLogger(Logger* logger, LogLevel log_level)
{
    loggers.push_back(make_pair(logger, Log::LevelToInt(log_level)));
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
    return ""; //Get Rid of Wanring.
}

Log::LogLevel Log::LogStringToLevel(const char* level)
{
    char* llevel = strdup(level);
    std::use_facet< std::ctype<char> >(std::locale("")).tolower(&llevel[0], &llevel[0] + strlen(llevel)); //Convert to lowercase.
    if(strcmp(llevel, "fatal") == 0)
        return Log::FATAL;
    else if(strcmp(llevel, "critical") == 0)
        return Log::CRITICAL;
    else if(strcmp(llevel, "warning") == 0)
        return Log::WARNING;
    else if(strcmp(llevel, "message") == 0)
        return Log::MESSAGE;
    else if(strcmp(llevel, "debug") == 0)
        return Log::DEBUG;
    else if(strcmp(llevel, "trace") == 0)
        return Log::TRACE;
    return Log::NONE;
}

void Log::vLog(int level, const char* format, va_list args)
{
    std::list< std::pair<Logger*, int> >::const_iterator i = loggers.begin();
    std::list< std::pair<Logger*, int> >::const_iterator l_end = loggers.end();
    if(i != l_end)
    {
        char messagestr[512];
        vsprintf(messagestr, format, args);

        do
        {
            if(i->second & level)
                i->first->Log(level, messagestr);
            i++;
        } while(i != l_end);
    }

    return;
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
    }

    return int_level;
}


void Log::CloseLoggers()
{
    std::list< std::pair<Logger*, int> >::const_iterator i = loggers.begin();
    std::list< std::pair<Logger*, int> >::const_iterator l_end = loggers.end();

    //Delete all Loggers
    while(i != l_end);
    {
        delete i->first;
        i++;
    }

    //Remove all References
    loggers.erase(loggers.begin(), loggers.end());

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
