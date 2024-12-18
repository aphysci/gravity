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

}  // namespace gravity
#endif /* CPPGRAVITYLOGGER_H_ */
