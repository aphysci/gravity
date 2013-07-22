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
 * GravityPublishManager.cpp
 *
 *  Created on: Oct 23, 2012
 *      Author: Mark Barger
 */

#include "CommUtil.h"
#include "GravityLogger.h"
#include "zmq.h"
#include <sstream>

namespace gravity
{

using namespace std;

GRAVITY_API void sendStringMessage(void* socket, string str, int flags)
{
	zmq_msg_t msg;
	zmq_msg_init_size(&msg, str.length());
	memcpy(zmq_msg_data(&msg), str.c_str(), str.length());
	zmq_sendmsg(socket, &msg, flags);
	zmq_msg_close(&msg);
}

GRAVITY_API string readStringMessage(void *socket)
{
	// Message holder
	zmq_msg_t msg;

	zmq_msg_init(&msg);
	zmq_recvmsg(socket, &msg, 0);
	int size = zmq_msg_size(&msg);
	char* s = (char*)malloc(size+1);
	memcpy(s, zmq_msg_data(&msg), size);
	s[size] = 0;
	std::string str(s, size);
	free(s);
	zmq_msg_close(&msg);

	return str;
}

GRAVITY_API void sendIntMessage(void* socket, int val, int flags)
{
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, sizeof(int));
    memcpy(zmq_msg_data(&msg), &val, sizeof(int));
    zmq_sendmsg(socket, &msg, flags);
    zmq_msg_close(&msg);
}

GRAVITY_API int readIntMessage(void *socket)
{
    // Message holder
    zmq_msg_t msg;

    zmq_msg_init(&msg);
    zmq_recvmsg(socket, &msg, 0);
    int size = zmq_msg_size(&msg);
    int val;
    memcpy(&val, zmq_msg_data(&msg), size);
    zmq_msg_close(&msg);

    return val;
}

GRAVITY_API void sendUint64Message(void* socket, uint64_t val, int flags)
{
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, sizeof(uint64_t));
    memcpy(zmq_msg_data(&msg), &val, sizeof(uint64_t));
    zmq_sendmsg(socket, &msg, flags);
    zmq_msg_close(&msg);
}

GRAVITY_API uint64_t readUint64Message(void* socket)
{
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(socket, &msg, 0);
    int size = zmq_msg_size(&msg);
    uint64_t val;
    memcpy(&val, zmq_msg_data(&msg), size);
    zmq_msg_close(&msg);

    return val;
}

GRAVITY_API void sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct, int flags)
{
    // Send data product
    zmq_msg_t data;
    zmq_msg_init_size(&data, dataProduct.getSize());
    dataProduct.serializeToArray(zmq_msg_data(&data));
    zmq_sendmsg(socket, &data, flags);
    zmq_msg_close(&data);
}


GRAVITY_API int bindFirstAvailablePort(void *socket, string ipAddr, int minPort, int maxPort)
{
    for (int i = minPort; i <= maxPort; i++) {
        stringstream ss;
        ss << "tcp://" << ipAddr << ":" << i;

        int rc = zmq_bind(socket, ss.str().c_str());
        if (rc == 0)
            return i;
    }
    return -1;
}
} /* namespace gravity */
