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

CXXTEST_ENUM_TRAITS( GravityReturnCode,
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::SUCCESS )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::FAILURE )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::NO_SERVICE_DIRECTORY )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::REQUEST_TIMEOUT )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::DUPLICATE )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::REGISTRATION_CONFLICT )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::NOT_REGISTERED )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::NO_SUCH_SERVICE )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::NO_SUCH_DATA_PRODUCT )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::LINK_ERROR )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::INTERRUPTED )
                     );

class GravityNodeTestSuite: public CxxTest::TestSuite, public GravitySubscriber,
													   public GravityServiceProvider,
													   public GravityRequestor {

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
    	pthread_mutex_init(&mutex, NULL);

        GravityReturnCode ret = node.init();

        ret = node.registerDataProduct("TEST", 5656, "tcp");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        // Set the subscribe & unsubscribe functionality
        // Give the consumer thread time to start up
        sleep(2);
        // Clear out subscription filled flag
        clearSubFlag();
        // Create and publish a message
        GravityDataProduct gdp("TEST");
        gdp.setFilterText("FILT");
        ret = node.publish(gdp);
        // Give it a couple secs
        sleep(2);
        // Check for subscription filled
        TS_ASSERT(subFilled());

        // Clear flag
        clearSubFlag();
        // Unsubscribe & wait a couple secs
        ret = node.unsubscribe("TEST", *this, "");
        sleep(2);
        // Resend message
        ret = node.publish(gdp);
        sleep(2);
        TS_ASSERT(!subFilled());

        // Test the request management
        node.registerService("SERVICE_TEST", 5757, "tcp", *this);
        sleep(2);
        node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::NO_SUCH_DATA_PRODUCT);

        /*
         *  try again after unregistering
         */
        ret = node.registerDataProduct("TEST", 5656, "tcp");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::NO_SUCH_DATA_PRODUCT);
    }

    void subscriptionFilled(const GravityDataProduct& dataProduct)
    {
    	pthread_mutex_lock(&mutex);
    	subFilledFlag = true;
    	pthread_mutex_unlock(&mutex);
    }

    void request(const GravityDataProduct& dataProduct)
    {
    	node.sendGravityDataProduct(socket, dataProduct);
    	pthread_mutex_lock(&mutex);
    	gotRequest = true;
    	pthread_mutex_unlock(&mutex);
    }

    void requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
    {
    	pthread_mutex_lock(&mutex);
    	gotResponse = (serviceID == "SERVICE_TEST" && requestID == "REQUEST_ID");
    	pthread_mutex_unlock(&mutex);
    }

    bool subFilled()
    {
    	bool ret;
    	pthread_mutex_lock(&mutex);
    	ret = subFilledFlag;
    	pthread_mutex_unlock(&mutex);
    	return ret;
    }

    void clearSubFlag()
    {
       	pthread_mutex_lock(&mutex);
       	subFilledFlag = false;
       	pthread_mutex_unlock(&mutex);
    }

    void clearReqRepFlags()
    {
    	pthread_mutex_lock(&mutex);
    	gotRequest = false;
    	gotResponse = false;
    	pthread_mutex_unlock(&mutex);
    }

    bool isRequestReceived()
    {
    	bool ret;
    	pthread_mutex_lock(&mutex);
    	ret = gotRequest;
    	pthread_mutex_unlock(&mutex);
    	return ret;
    }

    bool isResponseReceived()
    {
    	bool ret;
    	pthread_mutex_lock(&mutex);
    	ret = gotResponse;
    	pthread_mutex_unlock(&mutex);
    	return ret;
    }

private:
    pid_t pid;
    int sdFd;
    char buffer[BUFFER_SIZE];

    pthread_mutex_t mutex;
    bool subFilledFlag;
    bool gotRequest;
    bool gotResponse;

    GravityNode node;
};

#endif /* GRAVITYNODETESTSUITE_H_ */
