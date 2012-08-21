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
#include "GravityDataProduct.h"
#include "cxxtest/TestSuite.h"
#include "TestUtil.h"

#define BUFFER_SIZE 1000

using namespace std;
using namespace gravity;

class GravityNodeTestSuite: public CxxTest::TestSuite, public GravitySubscriber {

public:
	void setUp() {
		//memset(buffer, 0, BUFFER_SIZE);
		pid = popen2("ServiceDirectory", NULL, &sdFd);
		//int nbytes = read(sdFd, buffer, BUFFER_SIZE);
		//cout << endl << "output: " << buffer << endl;
	}

	void tearDown() {
		popen2("pkill -f \"^ServiceDirectory$\"", NULL, NULL);
	}

	void testRegister(void) {
		GravityNode* node = new GravityNode();
		GravityReturnCode ret = node->init();
		
		ret = node->registerDataProduct("TEST", 5656, "tcp");
		TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

		ret = node->subscribe("TEST", *this, "");
		TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
	}

	void subscriptionFilled(string dataProductID, vector<shared_ptr<GravityDataProduct> >) {}

private:
	pid_t pid;
	int sdFd;
	char buffer[BUFFER_SIZE];
};

#endif /* GRAVITYNODETESTSUITE_H_ */
