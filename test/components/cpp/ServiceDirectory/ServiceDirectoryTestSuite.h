/*
 * ServiceDirectoryTestSuite.h
 *
 *  Created on: Aug 14, 2012
 *      Author: Mark Barger
 */

#ifndef SERVICEDIRECTORYTESTSUITE_H_
#define SERVICEDIRECTORYTESTSUITE_H_

#include <iostream>
#include "cxxtest/TestSuite.h"

using namespace std;

class ServiceDirectoryTestSuite : public CxxTest::TestSuite {

public:
	void setUp()
	{
	}

	void tearDown()
	{
	}

    void testAddition(void)
    {
        TS_ASSERT(1 + 1 > 1);
        TS_ASSERT_EQUALS(1 + 1, 2);
    }
};

#endif /* SERVICEDIRECTORYTESTSUITE_H_ */
