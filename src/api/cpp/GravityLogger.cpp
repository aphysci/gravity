#include "GravityNode.h"
#include "GravityLogger.h"
#include "GravityLogMessagePB.pb.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace gravity;

//Initialize static data
FILE* Log::log_file = NULL;
int Log::current_local_levels = FATAL | CRITICAL | WARNING;
int Log::current_network_levels = FATAL | CRITICAL | WARNING;
string Log::log_dataProductID = "GRAVITY_LOGGER";
GravityNode* Log::gravity_node = NULL;


void Log::init(GravityNode *gn, const char* filename, unsigned short port, LogLevel local_log_level, LogLevel net_log_level)
{
    setLocalLevel(local_log_level);
    setNetworkLevel(net_log_level);

    gravity_node = gn;

    log_file = fopen(filename, "a");
    if(log_file == NULL)
    {
        log_file = fopen("/dev/null", "a");
        cerr << "[Log::init] Could not open log file: " << filename << endl;
    }

//    if(gn != NULL)
//    {
//        if(gravity_node.registerDataProduct(string(log_dataProductID), port, string("tcp")) != GravityReturnCodes::SUCCESS)
//        {
//            cerr << "[Log::init] Could not register Logger" << endl;
//            fprintf(log_file, "[Log::init] Could not register Logger\n");
//        }
//    }
//    else
//        current_network_levels = 0;
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
}

void Log::vLog(int level, const char* format, va_list args)
{
    if(current_local_levels & level)
    {
        //Format the Logs nicely.
        char timestr[100];
        time_t rawtime;
        struct tm * timeinfo;

        time ( &rawtime );
        timeinfo = localtime( &rawtime );

        strftime(timestr, 100, "%m/%d/%y %H:%M:%S", timeinfo);

        fprintf(log_file, "[%s %s] ", LogLevelToString((LogLevel)level), timestr); //NOTE: time is only needed for logging to a network file.

        char messagestr[500];
        vsprintf(messagestr, format, args);
        fputs(messagestr, log_file);
        fputs("\n", log_file);

        if(current_network_levels & level)
        {
            //TODO: how is this supposed to work???
//            gravity::GravityLogMessagePB log_message;
//            log_message.set_domain(domain);
//            log_message.set_level(LogLevelToString(level));
//            log_message.set_message(messagestr);
//
//            GravityDataProduct dp(log_dataProductID);
//            dp.setData(log_message);
//
//            gravity_node.publish(dp);
        }
    }

    return;
}


void Log::setLocalLevel(LogLevel level)
{
    switch(level)
    {
        case FATAL:
            current_local_levels = FATAL;
            break;
        case CRITICAL:
            current_local_levels = FATAL | CRITICAL;
            break;
        case WARNING:
            current_local_levels = FATAL | CRITICAL | WARNING;
            break;
        case MESSAGE:
            current_local_levels = FATAL | CRITICAL | WARNING | MESSAGE;
            break;
        case DEBUG:
            current_local_levels = FATAL | CRITICAL | WARNING | MESSAGE | DEBUG;
            break;
        case TRACE:
            current_local_levels = FATAL | CRITICAL | WARNING | MESSAGE | DEBUG | TRACE;
            break;
    }
}

void Log::setNetworkLevel(LogLevel level)
{
    switch(level)
    {
        case FATAL:
            current_network_levels = FATAL;
            break;
        case CRITICAL:
            current_network_levels = FATAL | CRITICAL;
            break;
        case WARNING:
            current_network_levels = FATAL | CRITICAL | WARNING;
            break;
        case MESSAGE:
            current_network_levels = FATAL | CRITICAL | WARNING | MESSAGE;
            break;
        case DEBUG:
            current_network_levels = FATAL | CRITICAL | WARNING | MESSAGE | DEBUG;
            break;
        case TRACE:
            current_network_levels = FATAL | CRITICAL | WARNING | MESSAGE | DEBUG | TRACE;
            break;
    }
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
