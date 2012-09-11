

#include "CPPGravitySubscriber.h"
#include <iostream>

using namespace gravity;

CPPGravitySubscriber::~CPPGravitySubscriber()
{}

void CPPGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
    cout << "made it into CPPGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)\n" << endl;
    signed char array[dataProduct.getSize()];
    subscriptionFilled(array, dataProduct.getSize());
}

void CPPGravitySubscriber::subscriptionFilled(const signed char* array, int length)
{
    cout << "made it into CPPGravitySubscriber::subscriptionFilled(const signed char* array, int length)\n" << endl;
}

