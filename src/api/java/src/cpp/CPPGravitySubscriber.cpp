

#include "CPPGravitySubscriber.h"
#include <iostream>

using namespace gravity;

CPPGravitySubscriber::~CPPGravitySubscriber()
{}

void CPPGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
    cout << "made it into CPPGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)\n" << endl;
    unsigned char array[dataProduct.getSize()];
    dataProduct.serializeToArray(array);
    subscriptionFilled((char*)array, dataProduct.getSize());
}

void CPPGravitySubscriber::subscriptionFilled(char* array, int length)
{
    cout << "made it into CPPGravitySubscriber::subscriptionFilled(const signed char* array, int length)\n" << endl;
}

