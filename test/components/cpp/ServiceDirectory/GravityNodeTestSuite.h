/*
 * GravityNodeTestSuite.h
 *
 *  Created on: Aug 17, 2012
 *      Author: Chris Brundick
 */

 /*
  * Gravity APIs Tested here:
  * registerDataProduct
  * unregisterDataProduct
  * subscribe
  * publish
  * registerService
  * unregisterService
  * request
  *
  * GravityDataProduct
  *
  * Gravity APIs NOT Tested here:
  * - Heart beat functionality: registerHeartbeatListener, startHeartbeat
  * - request (Synchronous)
  * - Config functionality (uses Sync request).  
  * - Logging
  * - getComponentID (very simple function)
  * - Connections over sockets on different machines (See ArchiverPlaybackTest)
  */
 
#ifndef GRAVITYNODETESTSUITE_H_
#define GRAVITYNODETESTSUITE_H_

#include <iostream>
#include "GravityNode.h"
#include "GravityLogger.h"
#include "GravityDataProduct.h"
#include "cxxtest/TestSuite.h"
#include "Utility.h"

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
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::LINK_ERROR )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::INTERRUPTED )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::NO_SERVICE_PROVIDER )
                     CXXTEST_ENUM_MEMBER( GravityReturnCodes::NO_PORTS_AVAILABLE )
                     );

class Subscriber : public GravitySubscriber
{
    int count;
public:
    Subscriber() : count(0) {}
    ~Subscriber() {}
    int getCount() { return count; }
    void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts) { count++; }
};

class GravityNodeTestSuite: public CxxTest::TestSuite, public GravitySubscriber,
															  GravityServiceProvider,
															  GravityRequestor {

public:
    void setUp()
    {
        pthread_mutex_init(&mutex, NULL);
        subFilledFlag = false;
        gotRequestFlag = false;
        gotResponseFlag = false;

        Log::initAndAddConsoleLogger(Log::MESSAGE);
    }

    void testRegisterData(void) {
        GravityNode node;
        GravityReturnCode ret = node.init("TestNode");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerDataProduct("TEST", "tcp");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerDataProduct("TEST", "tcp");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        /*
         *  try again after unregistering
         */
        ret = node.registerDataProduct("TEST", "tcp");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node.subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    }

    void testSubscriptionManager(void)
    {
    	GravityNode node;
    	GravityReturnCode ret = node.init("TestNode2");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        Subscriber preSubscriber;
        ret = node.subscribe("TEST", preSubscriber, "FILT");

        // Give the consumer thread time to start up
        sleep(20);

    	ret = node.registerDataProduct("TEST", "tcp");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        // Create and publish a message
        GravityDataProduct gdp("TEST");
        ret = node.publish(gdp, "FILTER");

        // Give the producer thread time to start up
        sleep(20);

        // Test that message was received by the old subscriber
        TS_ASSERT_EQUALS(preSubscriber.getCount(), 1);

        Subscriber postSubscriber;
    	ret = node.subscribe("TEST", postSubscriber, "FILT");

        // Give the consumer thread time to start up
        sleep(20);

        // Test that old message was received
        TS_ASSERT_EQUALS(postSubscriber.getCount(), 1);

        // publish a message again
        ret = node.publish(gdp, "FILTER");

        // Give the consumer thread time to start up
        sleep(20);

        TS_ASSERT_EQUALS(postSubscriber.getCount(), 2);

        // Clear out subscription filled flag
        clearSubFlag();

    	ret = node.subscribe("TEST", *this, "FILT");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    	// Give it a couple secs
    	sleep(20);

    	// Check for subscription filled
    	TS_ASSERT(subFilled());

    	// Check that postSubscriber wasn't called again
        TS_ASSERT_EQUALS(postSubscriber.getCount(), 2);

    	// Clear flag
    	clearSubFlag();

        // Resend message
        ret = node.publish(gdp, "FIL");

        // Give it a couple secs
        sleep(20);

        // Since full filter text isn't there, sub should not be filled
        TS_ASSERT(!subFilled());

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
    	GravityNode node;
    	GravityReturnCode ret = node.init("TestNode3");
    	//sleep(2);
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    	GravityDataProduct gdp("REQUEST");
    	gdp.setData("REQ_DATA", 9);

    	clearServiceFlags();

    	// Submit request to the service before it's available
    	ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::NO_SUCH_SERVICE);
    	sleep(2);

        // Register a service
        ret = node.registerService("SERVICE_TEST", "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
        sleep(2);

        // Submit request to the service
        ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
        sleep(2);

    	shared_ptr<GravityDataProduct> retGDP = node.request("SERVICE_TEST", gdp);
    	TS_ASSERT_EQUALS(retGDP->getDataProductID(), "RESPONSE");

    	ret = node.unregisterService("SERVICE_TEST");
    	TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    	// Check for request
    	TS_ASSERT(gotRequest());

    	// Check for response
    	TS_ASSERT(gotResponse());
    }

    void testRegisterService(void)
    {
	GravityNode* node1 = new GravityNode();
        GravityReturnCode ret = node1->init("TestNode4a");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
        ret = node1->registerService("TEST2", "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
	delete node1;
	
        GravityNode node;
        ret = node.init("TestNode4");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerService("TEST2", "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerService("TEST2", "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterService("TEST2");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerService("TEST2", "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.registerService("TEST2", "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node.unregisterService("TEST2");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    }

    void testDataProduct(void)
    {
    	GravityDataProduct gdp("GDP_ID");
    	gdp.setData("TEST_DATA", 10);

    	char* data = (char*)malloc(gdp.getSize());
    	gdp.serializeToArray(data);

    	GravityDataProduct gdp2(data, gdp.getSize());
    	TS_ASSERT(strcmp(gdp2.getDataProductID().c_str(), "GDP_ID")==0);
    	char* data2 = (char*)malloc(gdp.getDataSize());
    	gdp2.getData(data2, gdp2.getDataSize());
    	TS_ASSERT(strcmp(data2, "TEST_DATA")==0);

    	delete data;
    	delete data2;
    }

    void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts)
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

    shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct)
    {
    	shared_ptr<GravityDataProduct> ret(new GravityDataProduct("RESPONSE"));
    	ret->setData("RESP_DATA", 10);

    	pthread_mutex_lock(&mutex);
    	char* data = (char*)malloc(dataProduct.getDataSize());
    	dataProduct.getData(data, dataProduct.getDataSize());
    	gotRequestFlag = (strcmp(dataProduct.getDataProductID().c_str(), "REQUEST")==0 &&
    	    						strcmp(data, "REQ_DATA")==0);
    	pthread_mutex_unlock(&mutex);

    	return ret;
    }

    void requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
    {
    	pthread_mutex_lock(&mutex);
    	char* data = (char*)malloc(response.getDataSize());
    	response.getData(data, response.getDataSize());
    	gotResponseFlag = (strcmp(response.getDataProductID().c_str(), "RESPONSE")==0 &&
    						strcmp(data, "RESP_DATA")==0);
    	pthread_mutex_unlock(&mutex);
    }

private:
    pthread_mutex_t mutex;
    bool subFilledFlag;
    bool gotRequestFlag;
    bool gotResponseFlag;
};

#endif /* GRAVITYNODETESTSUITE_H_ */
