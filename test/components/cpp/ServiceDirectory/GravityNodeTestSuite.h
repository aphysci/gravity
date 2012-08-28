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
        cout << "starting SD" << endl;
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

class GravityNodeTestSuite: public CxxTest::TestSuite, public GravitySubscriber, GravityServiceProvider {

public:
    void testRegisterData(void) {
    	pthread_mutex_init(&mutex, NULL);

        GravityNode* node = new GravityNode();
        GravityReturnCode ret = node->init();
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->registerDataProduct("TEST", 5656, "tcp");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        // Set the subscribe & unsubscribe functionality
        // Give the consumer thread time to start up
        sleep(2);
        // Clear out subscription filled flag
        clearSubFlag();
        // Create and publish a message
        GravityDataProduct gdp("TEST");
        gdp.setFilterText("FILT");
        ret = node->publish(gdp);
        // Give it a couple secs
        sleep(2);
        // Check for subscription filled
        TS_ASSERT(subFilled());

        // Clear flag
        clearSubFlag();
        // Unsubscribe & wait a couple secs
        ret = node->unsubscribe("TEST", *this, "");
        sleep(2);
        // Resend message
        ret = node->publish(gdp);
        sleep(2);
        TS_ASSERT(!subFilled());

        ret = node->unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node->subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::NO_SUCH_DATA_PRODUCT);

        /*
         *  try again after unregistering
         */
        ret = node->registerDataProduct("TEST", 5656, "tcp");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->unregisterDataProduct("TEST");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node->subscribe("TEST", *this, "");
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::NO_SUCH_DATA_PRODUCT);

        delete node;
    }

    void testRegisterService(void)
    {
        GravityNode* node = new GravityNode();
        GravityReturnCode ret = node->init();
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->registerService("TEST2", 5657, "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->registerService("TEST2", 5657, "tcp", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        ret = node->unregisterService("TEST2", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

        ret = node->unregisterService("TEST2", *this);
        TS_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

        delete node;
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

    void request(const GravityDataProduct& dataProducts) {}

private:
    char buffer[BUFFER_SIZE];

    pthread_mutex_t mutex;
    bool subFilledFlag;
};

#endif /* GRAVITYNODETESTSUITE_H_ */
