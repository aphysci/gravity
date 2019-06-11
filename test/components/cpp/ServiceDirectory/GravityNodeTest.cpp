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

#include "GravityNodeTest.h"
#include "GravityTest.h"

#include <mutex>

namespace {
  std::mutex mtx;
} //end anonymous namespace

using namespace gravity;
using namespace std;

class Subscriber : public GravitySubscriber
{
    int count;
public:
    Subscriber() : count(0) {}
    ~Subscriber() {}
    int getCount() { return count; }
    void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts) { count++; }
};

class GravitySyncTest : public GravitySubscriber
{
    GravityNode gravityNode;
    GravityDataProduct gdp;
    int gdpcount1, gdpcount2;

public:

    GravitySyncTest();
    void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts);
    void testSync();
};

GravitySyncTest::GravitySyncTest() : gdp("SyncTestGDP")
{
}

void GravitySyncTest::testSync()
{
    gdpcount1 = gdpcount2 = 0;
    gravityNode.init("TestSync");
    gravityNode.registerDataProduct("SyncTestGDP", GravityTransportTypes::TCP);
    gravityNode.registerDataProduct("SyncTestGDP2", GravityTransportTypes::TCP);
    gravityNode.subscribe("SyncTestGDP", *this);
    gravityNode.subscribe("SyncTestGDP2", *this);

    // give it a second to setup subscriptions
    sleep(1000);

    for (int i = 0; i < 100; i++)
    {
        gdp.setData(&i, sizeof(int));
        gravityNode.publish(gdp);
        // mix up the timing a little
        if (i % 3 == 0)
            sleep(1);
    }

    // give it a second to finish sending subscription data
    sleep(1000);

    GRAVITY_TEST_EQUALS(gdpcount1, 100);
    GRAVITY_TEST_EQUALS(gdpcount2, 100);

    gravityNode.unsubscribe("SyncTestGDP", *this);
    gravityNode.unsubscribe("SyncTestGDP2", *this);
}

void GravitySyncTest::subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts)
{
    for(vector<std::shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
    {
        std::shared_ptr<GravityDataProduct> dataProduct = *i;
        if (dataProduct->getDataProductID().compare("SyncTestGDP") == 0)
        {
            gdpcount1++;
            // only compare these at the end when we know they should be the same
            if (gdpcount1 == 100)
            {
                GRAVITY_TEST(*dataProduct == gdp);
            }
            GravityDataProduct gdp2("SyncTestGDP2");
            gdp.setData(&gdpcount1, sizeof(int));
            gravityNode.publish(gdp2);
            GRAVITY_TEST(!(*dataProduct == gdp2));
        }
        else
        {
            gdpcount2++;
        }
    }
}

void GravityNodeTest::setUp()
{
    subFilledFlag = false;
    gotRequestFlag = false;
    gotResponseFlag = false;
}

void GravityNodeTest::testServiceWithDomain(void)
{
	GravityNode node;
	GravityReturnCode ret = node.init("TestDomainServiceNode");
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	// Get the domain from the Service Directory
	string domain;
	GravityDataProduct request("GetDomain");
	std::shared_ptr<GravityDataProduct> response = node.request("DirectoryService", request, 1000);
	GRAVITY_TEST(response);
	char* p = (char*)calloc(response->getDataSize(), sizeof(char));
	response->getData(p, response->getDataSize());
	domain.assign(p, response->getDataSize());	
	free(p);	
	GRAVITY_TEST_EQUALS(strcmp(domain.c_str(), "GravityTest"), 0);

	GravityDataProduct gdp("REQUEST");
	gdp.setData("REQ_DATA", 9);

	clearServiceFlags();

    // Register a service
    ret = node.registerService("SERVICE_TEST", GravityTransportTypes::TCP, *this);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
    sleep(2);

    // Submit async request to the service with domain specified
    ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID", -1, domain);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
    sleep(2);

	// Check for request
	GRAVITY_TEST(gotRequest());

	// Check for response
	GRAVITY_TEST(gotResponse());	

	// Submit sync request  with domain specified
	std::shared_ptr<GravityDataProduct> retGDP = node.request("SERVICE_TEST", gdp, -1, domain);
	GRAVITY_TEST_EQUALS(retGDP->getDataProductID(), "RESPONSE");

	// Clear service info
	clearServiceFlags();

	// Submit async request to the service with bad domain specified
    ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID", -1, domain+"_");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::NO_SUCH_SERVICE);
    sleep(2);

	// Check for request (should not be received)
	GRAVITY_TEST(!gotRequest());

	// Check for response (should not be received)
	GRAVITY_TEST(!gotResponse());

	// Clear service info
	clearServiceFlags();

	// Unregister service
	ret = node.unregisterService("SERVICE_TEST");
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
}

void GravityNodeTest::testSubscribeDomain(void)
{
	GravityNode node;
    GravityReturnCode ret = node.init("TestNode");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	// Get the domain from the Service Directory
	string domain;
	GravityDataProduct request("GetDomain");
	std::shared_ptr<GravityDataProduct> response = node.request("DirectoryService", request, 1000);
	GRAVITY_TEST(response);
	char* p = (char*)calloc(response->getDataSize(), sizeof(char));
	response->getData(p, response->getDataSize());
	domain.assign(p, response->getDataSize());	
	free(p);	
	GRAVITY_TEST_EQUALS(strcmp(domain.c_str(), "GravityTest"), 0);

	ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	Subscriber badSubscriber;
	ret = node.subscribe("TEST", badSubscriber, "", domain+"_");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	Subscriber goodSubscriber1;
	ret = node.subscribe("TEST", goodSubscriber1, "", domain);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	Subscriber goodSubscriber2;
	ret = node.subscribe("TEST", goodSubscriber2);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	// Give the consumer threads time to start up
    sleep(50);

	ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    // Create and publish a message
    GravityDataProduct gdp("TEST");
    ret = node.publish(gdp);

	// Give the consumers a little time
    sleep(50);

	// Test that the domains functioned properly
    GRAVITY_TEST_EQUALS(badSubscriber.getCount(), 0);
	GRAVITY_TEST_EQUALS(goodSubscriber1.getCount(), 1);
	GRAVITY_TEST_EQUALS(goodSubscriber2.getCount(), 1);

    node.unsubscribe("TEST", badSubscriber, "", domain+"_");
    node.unsubscribe("TEST", goodSubscriber1, "", domain);
    node.unsubscribe("TEST", goodSubscriber2);
}

void GravityNodeTest::testRegisterData(void)
{
    GravityNode node;
    GravityReturnCode ret = node.init("TestNode");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    /*
     *  try again after unregistering
     */
    ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    node.unsubscribe("TEST", *this, "");
}

void GravityNodeTest::testSubscriptionManager(void)
{
	GravityNode node;
	GravityReturnCode ret = node.init("TestNode2");
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    Subscriber preSubscriber;
    ret = node.subscribe("TEST", preSubscriber, "FILT");

    // Give the consumer thread time to start up
    sleep(20);

	ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    // Create and publish a message
    GravityDataProduct gdp("TEST");
    ret = node.publish(gdp, "FILTER");

    // Give the producer thread time to start up
    sleep(20);

    // Test that message was received by the old subscriber
    GRAVITY_TEST_EQUALS(preSubscriber.getCount(), 1);

    Subscriber postSubscriber;
	ret = node.subscribe("TEST", postSubscriber, "FILT");

    // Give the consumer thread time to start up
    sleep(20);

    // Test that old message was received
    GRAVITY_TEST_EQUALS(postSubscriber.getCount(), 1);

    // publish a message again
    ret = node.publish(gdp, "FILTER");

    // Give the consumer thread time to start up
    sleep(20);

    GRAVITY_TEST_EQUALS(postSubscriber.getCount(), 2);

    // Clear out subscription filled flag
    clearSubFlag();

	ret = node.subscribe("TEST", *this, "FILT");
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	// Give it a couple secs
	sleep(20);

	// Check for subscription filled
	GRAVITY_TEST(subFilled());

	// Check that postSubscriber wasn't called again
    GRAVITY_TEST_EQUALS(postSubscriber.getCount(), 2);

	// Clear flag
	clearSubFlag();

    // Resend message
    ret = node.publish(gdp, "FIL");

    // Give it a couple secs
    sleep(20);

    // Since full filter text isn't there, sub should not be filled
    GRAVITY_TEST(!subFilled());

    // Clear flag
    clearSubFlag();

	// Unsubscribe & wait a couple secs
	ret = node.unsubscribe("TEST", *this, "FILT");
	sleep(2);

	// Resend message
	ret = node.publish(gdp, "FILT");
	sleep(2);

	// Check to ensure that nothing was sent
	GRAVITY_TEST(!subFilled());

    node.unsubscribe("TEST", preSubscriber, "FILT");
    node.unsubscribe("TEST", postSubscriber, "FILT");
}

void GravityNodeTest::testServiceManager(void)
{
	GravityNode node;
	GravityReturnCode ret = node.init("TestNode3");
	//sleep(2);
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	GravityDataProduct gdp("REQUEST");
	gdp.setData("REQ_DATA", 9);

	clearServiceFlags();

	// Submit request to the service before it's available
	ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::NO_SUCH_SERVICE);
	sleep(2);

    // Register a service
    ret = node.registerService("SERVICE_TEST", GravityTransportTypes::TCP, *this);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
    sleep(2);

    // Submit request to the service
    ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
    sleep(2);

    std::shared_ptr<GravityDataProduct> retGDP = node.request("SERVICE_TEST", gdp);
	GRAVITY_TEST_EQUALS(retGDP->getDataProductID(), "RESPONSE");

	ret = node.unregisterService("SERVICE_TEST");
	GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

	// Check for request
	GRAVITY_TEST(gotRequest());

	// Check for response
	GRAVITY_TEST(gotResponse());
}

void GravityNodeTest::testRegisterService(void)
{
    GravityNode* node1 = new GravityNode();
    GravityReturnCode ret = node1->init("TestNode4a");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
    ret = node1->registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
    delete node1;

    GravityNode node;
    ret = node.init("TestNode4");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterService("TEST2");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterService("TEST2");
    GRAVITY_TEST_EQUALS(ret, GravityReturnCodes::SUCCESS);
}

void GravityNodeTest::testDataProduct(void)
{
	GravityDataProduct gdp("GDP_ID");
	gdp.setData("TEST_DATA", 10);

	char* data = (char*)malloc(gdp.getSize());
	gdp.serializeToArray(data);

	GravityDataProduct gdp2(data, gdp.getSize());
	GRAVITY_TEST(strcmp(gdp2.getDataProductID().c_str(), "GDP_ID")==0);
	char* data2 = (char*)malloc(gdp.getDataSize());
	gdp2.getData(data2, gdp2.getDataSize());
	GRAVITY_TEST(strcmp(data2, "TEST_DATA")==0);

	free(data);
	free(data2);
}

void GravityNodeTest::testComponentID(void)
{
    GravityNode gn;
    GRAVITY_TEST_EQUALS(gn.getComponentID(), "");
    gn.init("TestCompId");
    GRAVITY_TEST_EQUALS(gn.getComponentID(), "TestCompId");
}

void GravityNodeTest::subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts)
{
    std::lock_guard<std::mutex> guard(mtx);
    subFilledFlag = true;
}

bool GravityNodeTest::subFilled()
{
	bool ret;
  std::lock_guard<std::mutex> guard(mtx);
	ret = subFilledFlag;
	return ret;
}

void GravityNodeTest::clearSubFlag()
{
    std::lock_guard<std::mutex> guard(mtx);
   	subFilledFlag = false;
}

void GravityNodeTest::clearServiceFlags()
{
  std::lock_guard<std::mutex> guard(mtx);
	gotResponseFlag = false;
	gotRequestFlag = false;
}

bool GravityNodeTest::gotRequest()
{
	bool ret = false;
  std::lock_guard<std::mutex> guard(mtx);
   	ret = gotRequestFlag;
   	return ret;
}

bool GravityNodeTest::gotResponse()
{
  	bool ret = false;
    std::lock_guard<std::mutex> guard(mtx);
   	ret = gotResponseFlag;
   	return ret;
}

std::shared_ptr<GravityDataProduct> GravityNodeTest::request(const std::string serviceID, const GravityDataProduct& dataProduct)
{
    std::shared_ptr<GravityDataProduct> ret(new GravityDataProduct("RESPONSE"));
	ret->setData("RESP_DATA", 10);

  std::lock_guard<std::mutex> guard(mtx);
	char* data = (char*)malloc(dataProduct.getDataSize());
	dataProduct.getData(data, dataProduct.getDataSize());
	gotRequestFlag = (strcmp(dataProduct.getDataProductID().c_str(), "REQUEST")==0 &&
	    						strcmp(data, "REQ_DATA")==0);

	return ret;
}

void GravityNodeTest::requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
{
  std::lock_guard<std::mutex> guard(mtx);
	char* data = (char*)malloc(response.getDataSize());
	response.getData(data, response.getDataSize());
	gotResponseFlag = (strcmp(response.getDataProductID().c_str(), "RESPONSE")==0 &&
						strcmp(data, "RESP_DATA")==0);
}

int main( int argc, char *argv[] )
{
    GravityNodeTest gnTest;
    printf("\nAbout to run test setup...\n\n");
    gnTest.setUp();
    printf("\nFinished setup, about to run testRegisterData.\n\n");
    gnTest.testRegisterData();
    printf("\nFinished testRegisterData, about to run testSubscriptionManager.\n\n");
    gnTest.testSubscriptionManager();
    printf("\nFinished testSubscriptionManager, about to run testServiceManager.\n\n");
    gnTest.testServiceManager();
    printf("\nFinished testServiceManager, about to run testRegisterService.\n\n");
    gnTest.testRegisterService();
    printf("\nFinished testRegisterService, about to run testDataProduct.\n\n");
    gnTest.testDataProduct();
    printf("\nFinished testDataProduct, about to run testSubscribeDomain.\n\n");
    gnTest.testSubscribeDomain();
    printf("\nFinished testSubscribeDomain, about to run testServiceWithDomain.\n\n");
    gnTest.testServiceWithDomain();
    printf("\nFinished testServiceWithDomain, about to run testComponentID.\n\n");
    gnTest.testComponentID();
    printf("\nFinished testComponentID.\n\n");

    GravitySyncTest syncTest;
    syncTest.testSync();

    return 0;
}
