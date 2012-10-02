

#include "CPPGravitySubscriber.h"
#include <iostream>

using namespace gravity;

CPPGravitySubscriber::~CPPGravitySubscriber()
{}

void CPPGravitySubscriber::subscriptionFilled(const GravityDataProduct& dataProduct)
{
    unsigned char array[dataProduct.getSize()];
    dataProduct.serializeToArray(array);
    subscriptionFilled((char*)array, dataProduct.getSize());
}

int CPPGravitySubscriber::subscriptionFilled(char* array, int length)
{
    return 0;
}

