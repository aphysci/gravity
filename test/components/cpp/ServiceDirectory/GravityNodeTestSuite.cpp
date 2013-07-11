#include "GravityNodeTestSuite.h"
#include "GravityTestSuite.h"

void GravityNodeTestSuite::setUp()
{
    pthread_mutex_init(&mutex, NULL);
    subFilledFlag = false;
    gotRequestFlag = false;
    gotResponseFlag = false;
}

void GravityNodeTestSuite::testRegisterData(void) 
{
    GravityNode node;
    GravityReturnCode ret = node.init("TestNode");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    /*
     *  try again after unregistering
     */
    ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterDataProduct("TEST");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::REGISTRATION_CONFLICT);

    ret = node.subscribe("TEST", *this, "");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
}

void GravityNodeTestSuite::testSubscriptionManager(void)
{
	GravityNode node;
	GravityReturnCode ret = node.init("TestNode2");
	GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    Subscriber preSubscriber;
    ret = node.subscribe("TEST", preSubscriber, "FILT");

    // Give the consumer thread time to start up
    sleep(20);

	ret = node.registerDataProduct("TEST", GravityTransportTypes::TCP);
	GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    // Create and publish a message
    GravityDataProduct gdp("TEST");
    ret = node.publish(gdp, "FILTER");

    // Give the producer thread time to start up
    sleep(20);

    // Test that message was received by the old subscriber
    GRAVITY_ASSERT_EQUALS(preSubscriber.getCount(), 1);

    Subscriber postSubscriber;
	ret = node.subscribe("TEST", postSubscriber, "FILT");

    // Give the consumer thread time to start up
    sleep(20);

    // Test that old message was received
    GRAVITY_ASSERT_EQUALS(postSubscriber.getCount(), 1);

    // publish a message again
    ret = node.publish(gdp, "FILTER");

    // Give the consumer thread time to start up
    sleep(20);

    GRAVITY_ASSERT_EQUALS(postSubscriber.getCount(), 2);

    // Clear out subscription filled flag
    clearSubFlag();

	ret = node.subscribe("TEST", *this, "FILT");
	GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

	// Give it a couple secs
	sleep(20);

	// Check for subscription filled
	GRAVITY_ASSERT(subFilled());

	// Check that postSubscriber wasn't called again
    GRAVITY_ASSERT_EQUALS(postSubscriber.getCount(), 2);

	// Clear flag
	clearSubFlag();

    // Resend message
    ret = node.publish(gdp, "FIL");

    // Give it a couple secs
    sleep(20);

    // Since full filter text isn't there, sub should not be filled
    GRAVITY_ASSERT(!subFilled());

    // Clear flag
    clearSubFlag();

	// Unsubscribe & wait a couple secs
	ret = node.unsubscribe("TEST", *this, "");
	sleep(2);

	// Resend message
	ret = node.publish(gdp);
	sleep(2);

	// Check to ensure that nothing was sent
	GRAVITY_ASSERT(!subFilled());
}

void GravityNodeTestSuite::testServiceManager(void)
{
	GravityNode node;
	GravityReturnCode ret = node.init("TestNode3");
	//sleep(2);
	GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

	GravityDataProduct gdp("REQUEST");
	gdp.setData("REQ_DATA", 9);

	clearServiceFlags();

	// Submit request to the service before it's available
	ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");
	GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::NO_SUCH_SERVICE);
	sleep(2);

    // Register a service
    ret = node.registerService("SERVICE_TEST", GravityTransportTypes::TCP, *this);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    sleep(2);

    // Submit request to the service
    ret = node.request("SERVICE_TEST", gdp, *this, "REQUEST_ID");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    sleep(2);

	shared_ptr<GravityDataProduct> retGDP = node.request("SERVICE_TEST", gdp);
	GRAVITY_ASSERT_EQUALS(retGDP->getDataProductID(), "RESPONSE");

	ret = node.unregisterService("SERVICE_TEST");
	GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

	// Check for request
	GRAVITY_ASSERT(gotRequest());

	// Check for response
	GRAVITY_ASSERT(gotResponse());
}

void GravityNodeTestSuite::testRegisterService(void)
{
    GravityNode* node1 = new GravityNode();
    GravityReturnCode ret = node1->init("TestNode4a");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    ret = node1->registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
    delete node1;

    GravityNode node;
    ret = node.init("TestNode4");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterService("TEST2");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.registerService("TEST2", GravityTransportTypes::TCP, *this);
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);

    ret = node.unregisterService("TEST2");
    GRAVITY_ASSERT_EQUALS(ret, GravityReturnCodes::SUCCESS);
}

void GravityNodeTestSuite::testDataProduct(void)
{
	GravityDataProduct gdp("GDP_ID");
	gdp.setData("TEST_DATA", 10);

	char* data = (char*)malloc(gdp.getSize());
	gdp.serializeToArray(data);

	GravityDataProduct gdp2(data, gdp.getSize());
	GRAVITY_ASSERT(strcmp(gdp2.getDataProductID().c_str(), "GDP_ID")==0);
	char* data2 = (char*)malloc(gdp.getDataSize());
	gdp2.getData(data2, gdp2.getDataSize());
	GRAVITY_ASSERT(strcmp(data2, "TEST_DATA")==0);

	delete data;
	delete data2;
}

void GravityNodeTestSuite::subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts)
{
    pthread_mutex_lock(&mutex);
    subFilledFlag = true;
    pthread_mutex_unlock(&mutex);
}

bool GravityNodeTestSuite::subFilled()
{
	bool ret;
	pthread_mutex_lock(&mutex);
	ret = subFilledFlag;
	pthread_mutex_unlock(&mutex);
	return ret;
}

void GravityNodeTestSuite::clearSubFlag()
{
   	pthread_mutex_lock(&mutex);
   	subFilledFlag = false;
   	pthread_mutex_unlock(&mutex);
}

void GravityNodeTestSuite::clearServiceFlags()
{
	pthread_mutex_lock(&mutex);
	gotResponseFlag = false;
	gotRequestFlag = false;
	pthread_mutex_unlock(&mutex);
}

bool GravityNodeTestSuite::gotRequest()
{
	bool ret = false;
  	pthread_mutex_lock(&mutex);
   	ret = gotRequestFlag;
   	pthread_mutex_unlock(&mutex);
   	return ret;
}

bool GravityNodeTestSuite::gotResponse()
{
  	bool ret = false;
  	pthread_mutex_lock(&mutex);
   	ret = gotResponseFlag;
  	pthread_mutex_unlock(&mutex);
   	return ret;
}

shared_ptr<GravityDataProduct> GravityNodeTestSuite::request(const std::string serviceID, const GravityDataProduct& dataProduct)
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

void GravityNodeTestSuite::requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
{
	pthread_mutex_lock(&mutex);
	char* data = (char*)malloc(response.getDataSize());
	response.getData(data, response.getDataSize());
	gotResponseFlag = (strcmp(response.getDataProductID().c_str(), "RESPONSE")==0 &&
						strcmp(data, "RESP_DATA")==0);
	pthread_mutex_unlock(&mutex);
}

int main( int argc, char *argv[] ) 
{
    GravityNodeTestSuite gnTest;
    gnTest.setUp();
    gnTest.testRegisterData();
    gnTest.testSubscriptionManager();
    gnTest.testServiceManager();
    gnTest.testRegisterService();
    gnTest.testDataProduct();
    return 0;
}