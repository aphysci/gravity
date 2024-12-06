/* (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
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
#include <memory>
#include <sys/time.h>
#else
#include <WinSock2.h>
#include <memory>
#endif
#include <vector>
#include <string>

#define MIN_PORT 24000
#define MAX_PORT 24999

#define DEFAULT_BROADCAST_RATE_SEC 10
#define DEFAULT_BROADCAST_PORT 8276
#define DEFAULT_BROADCAST_TIMEOUT_SEC 10
#define MAXRECVSTRING 255

namespace gravity
{
namespace constants
{
    // ServiceDirectory Reserved Data Products
    const std::string REGISTERED_PUBLISHERS_DPID = "RegisteredPublishers";
    const std::string DOMAIN_DETAILS_DPID = "ServiceDirectory_DomainDetails";
    const std::string DOMAIN_UPDATE_DPID = "ServiceDirectory_DomainUpdate";
    const std::string DIRECTORY_SERVICE_DPID = "DirectoryService";

    // GravityNode Reserved Data Products
    const std::string METRICS_DATA_DPID = "GravityMetricsData";
    const std::string GRAVITY_SETTINGS_DPID = "GRAVITY_SETTINGS";
    const std::string GRAVITY_LOGGER_DPID = "GRAVITY_LOGGER";
}  // namespace constants

/**
 * @name ZMQ Socket functions 
 * @{
 *  Functions that work with ZMQ sockets by sending messages,
 *  receiving messages, or binding sockets to a port.
 *
 *  All functions are wrappers around zmq_recvmsg(), zmq_sendmsg(), or zmq_bind().
 *  \param socket zmq_socket
 */

/**
 * Read a message through the zmq socket.
 * \ref readStringMessage(void*,int) with 0 as the flag
 * \note flag 0 is not defined in zmq.h. From the unit tests it seems that this will block until it receives a message. If messages are sent with the ZMQ_SNDMORE flag, zmq will hold off on actually sending the messages until it has the final message with the ZMQ_DONTWAIT flag. Until then this will be a blocking call.
 * \return message
 */
GRAVITY_API std::string readStringMessage(void* socket);

/**
 * \copybrief readStringMessage(void*)
 * \param flags a zmq flag. Options include:
 * \n ZMQ_DONTWAIT (set to 1 in zmq.h) "Specifies that the operation should be performed in non-blocking mode."
 * \n <a href="http://api.zeromq.org/3-0:zmq-recvmsg">source</a>
 * \copydetails readStringMessage(void*)
 */
GRAVITY_API std::string readStringMessage(void* socket, int flags);

/**
 * Send a message through a zmq socket
 * \param flags a zmq flag. Options include: 
 * \n ZMQ_DONTWAIT (set to 1 in zmq.h) "Specifies that the operation should be performed in non-blocking mode."
 * \n ZMQ_SNDMORE (set to 2 in zmq.h) "Specifies that the message being sent is a multi-part message, and that further message data parts are to follow." 
 * \n <a href="http://api.zeromq.org/3-0:zmq-sendmsg">source</a>
 */
GRAVITY_API void sendStringMessage(void* socket, std::string str, int flags);
GRAVITY_API int readIntMessage(void* socket);  ///< \copydoc readStringMessage(void*)
GRAVITY_API void sendIntMessage(void* socket, int val,
                                int flags);            ///< \copydoc sendStringMessage(void*,std::string,int)
GRAVITY_API uint64_t readUint64Message(void* socket);  ///< \copydoc readStringMessage(void*)
GRAVITY_API void sendUint64Message(void* socket, uint64_t val,
                                   int flags);         ///< \copydoc sendStringMessage(void*,std::string,int)
GRAVITY_API uint32_t readUint32Message(void* socket);  ///< \copydoc readStringMessage(void*)
GRAVITY_API void sendUint32Message(void* socket, uint32_t val,
                                   int flags);  ///< \copydoc sendStringMessage(void*,std::string,int)

/**
 * \copydoc sendStringMessage(void*,std::string,int)
 * \return The number of bytes in the message if successful. Otherwise it shall return -1.
 */
GRAVITY_API int sendGravityDataProduct(void* socket, const GravityDataProduct& dataProduct, int flags);
GRAVITY_API int sendProtobufMessage(
    void* socket, const google::protobuf::Message& pb,
    int flags);  ///< \copydoc sendGravityDataProduct(void*,const GravityDataProduct&,int)

/**
 * Bind the given zmq socket to the first available port.
 * \return zero if successfully bound to a port. Otherwise it shall return -1.
 * \n <a href="http://api.zeromq.org/3-2:zmq-bind">source</a>
 */
GRAVITY_API int bindFirstAvailablePort(void* socket, std::string ipAddr, int minPort, int maxPort);
/** @} */  //ZMQ Socket functions

/**
 * @name Time functions
 * @{
 *  Some of the following functions work with timeval, 
 *  which is defined in <sys/time.h> on Linux systems as: 
 *  \verbatim
    struct timeval {
        time_t       tv_sec;  // seconds 
        suseconds_t  tv_usec; // microseconds 
    };
    \endverbatim
 *  Where tv_sec + tv_usec / 10^6 is the total number of seconds.
 *  Time is in Unix epoch.
 */
#ifdef _WIN32
/**
 * An equivalent function to Linux's gettimeofday() to run on Windows. 
 * Get the current time since Unix epoch and place in \param tp
 * \param tp timeval structure that this function modifies 
 * \param tzp UNUSED - always returns time in UTC
 * \return 0 always
 */
GRAVITY_API int gettimeofday(struct timeval* tp, struct timezone* tzp);
#endif

/**
 * Add the given second to the current time. 
 * \param t1 timeval structure (will not be unmodified)
 * \param sec seconds to add 
 * \return a timeval structure with the added seconds 
 */
GRAVITY_API struct timeval addTime(const struct timeval* t1, int sec);

/**
 * Compare two times.
 * \return 0 if they are equal, -1 if t1 < t2, and 1 if t1 > t2 
 */
GRAVITY_API int timevalcmp(const struct timeval* t1, const struct timeval* t2);

/**
 * Return the time difference.
 * \return a timeval structure equivalent to t1 - t2. If t1 > t2, return a timeval structure with time set to 0.  
 */
GRAVITY_API struct timeval subtractTime(const struct timeval* t1, const struct timeval* t2);

/**
 * Return time in milliseconds.
 */
GRAVITY_API unsigned int timevalToMilliSeconds(const struct timeval* tv);
/** @} */  //Time functions

/**
 * Print the char array to the console.
 */
GRAVITY_API void printByteBuffer(const char* buffer, int len);

} /* namespace gravity */

#endif /* COMMUTIL_H_ */
