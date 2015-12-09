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

/*
 * GravitySubscriptionManager.h
 *
 *  Created on: Oct 26, 2012
 *      Author: Mark Barger
 */

#ifndef COMMUTIL_H_
#define COMMUTIL_H_

#include "GravityDataProduct.h"

#ifdef __GNUC__
#include <tr1/memory>
#include <sys/time.h>
#else
#include <WinSock2.h>
#include <memory>
#endif
#include <vector>
#include <string>

#define MIN_PORT 24000
#define MAX_PORT 24999

#define DEFAULT_BROADCAST_RATE_SEC  10
#define	DEFAULT_BROADCAST_PORT   8276
#define DEFAULT_BROADCAST_TIMEOUT_SEC 10 
#define MAXRECVSTRING 255

namespace gravity
{

using namespace std;
using namespace std::tr1;

GRAVITY_API string readStringMessage(void *socket);
GRAVITY_API void sendStringMessage(void* socket, string str, int flags);
GRAVITY_API int readIntMessage(void *socket);
GRAVITY_API void sendIntMessage(void* socket, int val, int flags);
GRAVITY_API uint64_t readUint64Message(void* socket);
GRAVITY_API void sendUint64Message(void* socket, uint64_t val, int flags);
GRAVITY_API uint32_t readUint32Message(void* socket);
GRAVITY_API void sendUint32Message(void* socket, uint32_t val, int flags);
GRAVITY_API void sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct, int flags);
GRAVITY_API int bindFirstAvailablePort(void *socket, string ipAddr, int minPort, int maxPort);
#ifdef _WIN32
GRAVITY_API int gettimeofday(struct timeval * tp, struct timezone * tzp);
#endif
GRAVITY_API struct timeval addTime(struct timeval *t1, int sec);
GRAVITY_API int timevalcmp(struct timeval* t1, struct timeval* t2);
GRAVITY_API struct timeval subtractTime(struct timeval *t1, struct timeval *t2);
GRAVITY_API unsigned int timevalToMilliSeconds(struct timeval *tv);
GRAVITY_API void printByteBuffer(char* buffer, int len);

} /* namespace gravity */

#endif /* COMMUTIL_H_ */
