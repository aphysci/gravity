/*
 * GravityNodeTest.h
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
 
#ifndef GRAVITYNODETEST_H_
#define GRAVITYNODETEST_H_

#include <iostream>
#include "GravityNode.h"
#include "GravityLogger.h"
#include "GravityDataProduct.h"
#include "Utility.h"

using namespace std;
using namespace gravity;

class Subscriber : public GravitySubscriber
{
    int count;
public:
    Subscriber() : count(0) {}
    ~Subscriber() {}
    int getCount() { return count; }
    void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts) { count++; }
};

class GravityNodeTest: public GravitySubscriber, GravityServiceProvider, GravityRequestor 
{
public:
    void setUp();
    void testRegisterData(void);
    void testSubscriptionManager(void);
    void testServiceManager(void);
    void testRegisterService(void);
    void testDataProduct(void);
    void subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts);
    void requestFilled(string serviceID, string requestID, const GravityDataProduct& response);
    shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);

private:
    pthread_mutex_t mutex;
    bool gotResponse();
    bool gotRequest();
    void clearServiceFlags();
    void clearSubFlag();
    bool subFilled();
    bool subFilledFlag;
    bool gotRequestFlag;
    bool gotResponseFlag;
};

#endif /* GRAVITYNODETEST_H_ */
