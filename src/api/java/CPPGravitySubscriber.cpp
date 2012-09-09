

#include "CPPGravitySubscriber.h"
#include <iostream>

using namespace gravity;

void CPPGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
    cout << "made it into CPPGravitySubscriber::subscriptionFilled\n" << endl;
}

void CPPGravitySubscriber::setJavaSubscriber(const GravitySubscriber& subscriber)
{
    cout << "made it into CPPGravitySubscriber::setJavaSubscriber\n" << endl;
}

