/*
 * ServiceDirectoryTestSuite.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Mark Barger
 */

#ifndef SERVICEDIRECTORYTESTSUITE_H_
#define SERVICEDIRECTORYTESTSUITE_H_

#include <iostream>
//#include "zmq.h"
#include "cxxtest/TestSuite.h"
#include "ServiceDirectoryRegistrationPB.pb.h"
#include "TestUtil.h"

#define BUFFER_SIZE 1000

using namespace std;
using namespace gravity;

class ServiceDirectoryTestSuite: public CxxTest::TestSuite {

public:
	void setUp() {
//		context = zmq_init(1);

		memset(buffer, 0, BUFFER_SIZE);
		pid = popen2("ServiceDirectory", NULL, &sdFd);
		int nbytes = read(sdFd, buffer, BUFFER_SIZE);
		cout << endl << "output: " << buffer << endl;
//		struct timeval tv = { 0,  1000000L};
//		select(0, NULL, NULL, NULL, &tv );
	}

	void tearDown() {
		popen2("pkill -f \"^ServiceDirectory$\"", NULL, NULL);
//		if (pid > 0) {
//			kill(pid, SIGTERM);
//		}
	}

	void testRegister(void) {
//		void *socket = zmq_socket(context, ZMQ_REQ);

		ServiceDirectoryRegistrationPB sdr;
		sdr.set_id("TestDataProduct1");
		sdr.set_url("TestUrl1");

//		int nbytes = read(sdFd, buffer, BUFFER_SIZE);
//		cout << endl << "output: " << buffer << endl;

	}

private:
	pid_t pid;
	int sdFd;
	char buffer[BUFFER_SIZE];
	void *context;
};

#endif /* SERVICEDIRECTORYTESTSUITE_H_ */
