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

class TestFixture : public CxxTest::GlobalFixture
{
public:
    bool setUpWorld()
    {
        //memset(buffer, 0, BUFFER_SIZE);
        cout << endl << "starting SD" << endl;
        pid = popen2("ServiceDirectory > sd.out", NULL, &sdFd);
        //int nbytes = read(sdFd, buffer, BUFFER_SIZE);
        //cout << endl << "output: " << buffer << endl;
        return true;
    }
    bool tearDownWorld()
    {
        popen2("pkill -f \"^ServiceDirectory$\"", NULL, NULL);
        return true;
    }
private:
    pid_t pid;
    int sdFd;
};
static TestFixture testFixture;

class GravityNodeTestSuite: public CxxTest::TestSuite, public GravitySubscriber,
															  GravityServiceProvider,
															  GravityRequestor {

public:
    void testRegisterData(void) {
    	pthread_mutex_init(&mutex, NULL);

        GravityNode node;
        GravityReturnCode ret = node.init();
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

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

    void testSubscriptionManager(void)
    {
    	pthread_mutex_init(&mutex, NULL);

    	GravityNode node;
    	GravityReturnCode ret = node.init();
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    	ret = node.registerDataProduct("TEST", 5656, "tcp");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    	ret = node.subscribe("TEST", *this, "");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

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

    	// Check to ensure that nothing was sent
    	TS_ASSERT(!subFilled());
    }

    void testServiceManager(void)
    {
    	pthread_mutex_init(&mutex, NULL);

    	GravityNode node;
    	GravityReturnCode ret = node.init();
    	//sleep(2);
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    	GravityDataProduct gdp("TEST");
    	gdp.setFilterText("FILT");

    	clearServiceFlags();

    	// Register a service
    	ret = node.registerService("SERVICE_TEST", 5757, "tcp", *this);
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    	sleep(2);

    	// Submit request to the service
    	ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    	sleep(2);

    	// Check for request
    	TS_ASSERT(gotRequest());

    	// Check for response
    	TS_ASSERT(gotResponse());
    }

    void testRegisterService(void)
    {
        GravityNode node;
        GravityReturnCode ret = node.init();
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerService("TEST2", 5657, "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerService("TEST2", 5657, "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node.unregisterService("TEST2");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterService("TEST2");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);
    }

    void subscriptionFilled(const GravityDataProduct& dataProduct)
    {
        pthread_mutex_lock(&mutex);
        subFilledFlag = true;
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

    void clearServiceFlags()
    {
    	pthread_mutex_lock(&mutex);
    	gotResponseFlag = false;
    	gotRequestFlag = false;
    	pthread_mutex_unlock(&mutex);
    }

    bool gotRequest()
    {
    	bool ret = false;
      	pthread_mutex_lock(&mutex);
       	ret = gotRequestFlag;
       	pthread_mutex_unlock(&mutex);
       	return ret;
    }

    bool gotResponse()
    {
      	bool ret = false;
      	pthread_mutex_lock(&mutex);
       	ret = gotResponseFlag;
      	pthread_mutex_unlock(&mutex);
       	return ret;
    }

    shared_ptr<GravityDataProduct> request(const GravityDataProduct& dataProduct)
    {
    	shared_ptr<GravityDataProduct> ret(new GravityDataProduct("RESPONSE"));
    	ret->setFilterText(dataProduct.getFilterText());

    	pthread_mutex_lock(&mutex);
    	gotRequestFlag = true;
    	pthread_mutex_unlock(&mutex);

    	return ret;
    }

    void requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
    {
    	pthread_mutex_lock(&mutex);
    	gotResponseFlag = true;
    	pthread_mutex_unlock(&mutex);
    }

private:
    char buffer[BUFFER_SIZE];

    pthread_mutex_t mutex;
    bool subFilledFlag;
    bool gotRequestFlag;
    bool gotResponseFlag;
};

#endif /* GRAVITYNODETESTSUITE_H_ */
