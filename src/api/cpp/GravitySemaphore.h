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

#ifndef ZMQ_SEMEPHORE_H__
#define ZMQ_SEMEPHORE_H__
#include <semaphore.h>
#include "Utility.h"

namespace gravity
{

/**
 * Used to process synchronization in this multi-processing environment.
 */
class Semaphore
{
public:
    GRAVITY_API Semaphore();
    GRAVITY_API Semaphore(int count);
    GRAVITY_API void Lock();
    GRAVITY_API void Unlock();
    GRAVITY_API ~Semaphore();

private:
    sem_t semaphore;
};

}  // namespace gravity

#endif  //ZMQ_SEMEPHORE_H__
