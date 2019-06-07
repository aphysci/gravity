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
	return readStringMessage(socket, 0);
}

GRAVITY_API string readStringMessage(void *socket, int flags)
{
	// Message holder
	zmq_msg_t msg;

	zmq_msg_init(&msg);
	zmq_recvmsg(socket, &msg, flags);
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

GRAVITY_API void sendUint32Message(void* socket, uint32_t val, int flags)
{
    zmq_msg_t msg;
    zmq_msg_init_size(&msg, sizeof(uint32_t));
    memcpy(zmq_msg_data(&msg), &val, sizeof(uint32_t));
    zmq_sendmsg(socket, &msg, flags);
    zmq_msg_close(&msg);
}

GRAVITY_API uint32_t readUint32Message(void* socket)
{
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    zmq_recvmsg(socket, &msg, 0);
    int size = zmq_msg_size(&msg);
    uint32_t val;
    memcpy(&val, zmq_msg_data(&msg), size);
    zmq_msg_close(&msg);

    return val;
}

GRAVITY_API int sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct, int flags)
{
    // Send data product
    zmq_msg_t data;
    zmq_msg_init_size(&data, dataProduct.getSize());
    dataProduct.serializeToArray(zmq_msg_data(&data));
    int rc = zmq_sendmsg(socket, &data, flags);
    zmq_msg_close(&data);

	return rc;
}

GRAVITY_API int sendProtobufMessage(void* socket, const google::protobuf::Message& pb, int flags)
{
    // Send data product
    zmq_msg_t data;
    zmq_msg_init_size(&data, pb.ByteSize());
    pb.SerializeToArray(zmq_msg_data(&data), pb.ByteSize());
    int rc = zmq_sendmsg(socket, &data, flags);
    zmq_msg_close(&data);

	return rc;
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

#ifdef _WIN32

GRAVITY_API int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}
#endif

GRAVITY_API struct timeval addTime(const struct timeval *t1, int sec)
{
	struct timeval newTime;
	newTime.tv_sec=t1->tv_sec+sec;
	newTime.tv_usec= t1->tv_usec;

	return newTime;
}

GRAVITY_API int timevalcmp(const struct timeval* t1, const struct timeval* t2)
{
	if (t1->tv_sec ==t2->tv_sec)
	{
		if(t1->tv_usec == t2->tv_usec)
		{
			return 0;
		}
		else if(t1->tv_usec < t2->tv_usec)
		{
			return -1;
		}
		else
		{
			return 1;
		}
	}
	else if(t1->tv_sec < t2->tv_sec)
	{
		return -1;
	}
	else
	{
		return 1;
	}
}

GRAVITY_API struct timeval subtractTime(const struct timeval *t1, const struct timeval *t2)
{
	struct timeval newTime;
	newTime.tv_sec=0;
	newTime.tv_usec=0;
	//make sure t1 is greater than t2
	if(timevalcmp(t1,t2) <=0)
	{
		return newTime;
	}

	newTime.tv_sec=t1->tv_sec - t2->tv_sec;
	long sub = t1->tv_usec;
	if(t2->tv_usec > t1->tv_usec)
	{
		sub = 1000000;
	}
	newTime.tv_usec=sub-t2->tv_usec;

	return newTime;
}

GRAVITY_API unsigned int timevalToMilliSeconds(const struct timeval *tv)
{
	return (unsigned int) (tv->tv_usec/1000) + (tv->tv_sec*1000);
}

GRAVITY_API void printByteBuffer(const char* buffer, int len)
{
	for(int i = 0; i < len; i++)
	{
		printf("%02X ",*(buffer+i));
	}
	printf("\n");
}

} /* namespace gravity */
