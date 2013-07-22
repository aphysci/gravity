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

#include "GravitySemaphore.h"
#include <zmq.h>
#include <sstream>
#include <pthread.h>

using namespace std;

namespace gravity {

Semaphore::Semaphore()
{
	sem_init(&semaphore, 0, 1);
}

Semaphore::Semaphore(int count)
{
	sem_init(&semaphore, 0, count);
}

void Semaphore::Lock()
{
	sem_wait(&semaphore);
}
void Semaphore::Unlock()
{
	sem_post(&semaphore);
}

Semaphore::~Semaphore()
{
	sem_destroy(&semaphore);
}

}//namespace gravity
