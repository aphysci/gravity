

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

    static void initAndAddFileLogger(const char* filename, LogLevel local_log_level);

    static void initAndAddConsoleLogger(LogLevel local_log_level);

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