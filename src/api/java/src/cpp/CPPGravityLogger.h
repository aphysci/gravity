
#ifndef CPPGRAVITYLOGGER_H_
#define CPPGRAVITYLOGGER_H_

#include "GravityLogger.h"

namespace gravity
{

/**
 * Native implementation of a Logger
 */
class CPPGravityLogger : public Logger
{
public:

    virtual ~CPPGravityLogger();
    virtual void Log(int level, const char* messagestr);
};

}
#endif /* CPPGRAVITYLOGGER_H_ */

