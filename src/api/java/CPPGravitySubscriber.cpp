

#include "CPPGravitySubscriber.h"
#include <iostream>

using namespace gravity;

void CPPGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
    cout << "made it into CPPGravitySubscriber::subscriptionFilled\n" << endl;
}


