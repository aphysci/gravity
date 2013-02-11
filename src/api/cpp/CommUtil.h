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
#else
#include <memory>
#endif
#include <vector>
#include <string>

#define MIN_PORT 24000
#define MAX_PORT 24999

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
GRAVITY_API void sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct, int flags);
GRAVITY_API int bindFirstAvailablePort(void *socket, string ipAddr, int minPort, int maxPort);

} /* namespace gravity */
#endif /* COMMUTIL_H_ */
