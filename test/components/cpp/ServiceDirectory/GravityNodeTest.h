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

class GravityNodeTest: public gravity::GravitySubscriber, gravity::GravityServiceProvider, gravity::GravityRequestor
{
public:
    void setUp();
    void testRegisterData(void);
    void testSubscriptionManager(void);
    void testServiceManager(void);
    void testRegisterService(void);
    void testDataProduct(void);
	void testSubscribeDomain(void);
	void testServiceWithDomain(void);
	void testComponentID(void);
    void subscriptionFilled(const std::vector< std::tr1::shared_ptr<gravity::GravityDataProduct> >& dataProducts);
    void requestFilled(std::string serviceID, std::string requestID, const gravity::GravityDataProduct& response);
    std::tr1::shared_ptr<gravity::GravityDataProduct> request(const std::string serviceID, const gravity::GravityDataProduct& dataProduct);

private:
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
