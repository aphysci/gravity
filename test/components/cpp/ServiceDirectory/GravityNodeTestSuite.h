/*
 * GravityNodeTestSuite.h
 *
 *  Created on: Aug 17, 2012
 *      Author: Chris Brundick
 */

#ifndef GRAVITYNODETESTSUITE_H_
#define GRAVITYNODETESTSUITE_H_

#include <iostream>
#include "GravityNode.h"
#include "cxxtest/TestSuite.h"
#include "TestUtil.h"

#define BUFFER_SIZE 1000

using namespace std;
using namespace gravity;

class GravityNodeTestSuite: public CxxTest::TestSuite {

public:
	void setUp() {
		cout << endl << "GravityNodeTestSuite.setUp()" << endl;

		memset(buffer, 0, BUFFER_SIZE);
		pid = popen2("ServiceDirectory", NULL, &sdFd);
		int nbytes = read(sdFd, buffer, BUFFER_SIZE);
		cout << endl << "output: " << buffer << endl;
	}

	void tearDown() {
		cout << "GravityNodeTestSuite.tearDown()" << endl;
		popen2("pkill -f \"^ServiceDirectory$\"", NULL, NULL);
	}

	void testRegister(void) {
		cout << "GravityNodeTestSuite.testRegister()" << endl;

		GravityNode* node = new GravityNode();
		GravityReturnCode ret = node->init();
		ret = node->registerDataProduct("TEST", 5656, "tcp");
		switch(ret)
		{
			case GravityReturnCodes::SUCCESS:
				cout << "test result = SUCCESS" << endl;
				break;
			case GravityReturnCodes::FAILURE:
				cout << "test result = FAILURE" << endl;
				break;
			case GravityReturnCodes::NO_SERVICE_DIRECTORY:
				cout << "test result = NO_SERVICE_DIRECTORY" << endl;
				break;
			case GravityReturnCodes::REQUEST_TIMEOUT:
				cout << "test result = REQUEST_TIMEOUT" << endl;
				break;
			case GravityReturnCodes::DUPLICATE:
				cout << "test result = DUPLICATE" << endl;
				break;
			case GravityReturnCodes::REGISTRATION_CONFLICT:
				cout << "test result = REGISTRATION_CONFLICT" << endl;
				break;
			case GravityReturnCodes::NO_SUCH_SERVICE:
				cout << "test result = NO_SUCH_SERVICE" << endl;
				break;
			case GravityReturnCodes::NO_SUCH_DATA_PRODUCT:
				cout << "test result = NO_SUCH_DATA_PRODUCT" << endl;
				break;
			case GravityReturnCodes::LINK_ERROR:
				cout << "test result = LINK_ERROR" << endl;
				break;
			case GravityReturnCodes::INTERRUPTED:
				cout << "test result = INTERRUPTED" << endl;
				break;
		}
	}

private:
	pid_t pid;
	int sdFd;
	char buffer[BUFFER_SIZE];
};

#endif /* GRAVITYNODETESTSUITE_H_ */
