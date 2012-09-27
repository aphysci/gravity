

#include "CPPGravityServiceProvider.h"
#include <iostream>

using namespace gravity;

CPPGravityServiceProvider::~CPPGravityServiceProvider()
{}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(const GravityDataProduct& dataProduct)
{
    cout << "made it into CPPGravityServiceProvider::request(const GravityDataProduct& dataProduct)\n" << endl;
    unsigned char array[dataProduct.getSize()];
    dataProduct.serializeToArray(array);
    request((char*)array, dataProduct.getSize());
    shared_ptr<GravityDataProduct> ret(new GravityDataProduct("RESPONSE"));
    return ret;
}

int CPPGravityServiceProvider::request(char* array, int length)
{
    cout << "made it into CPPGravityServiceProvider::request(const signed char* array, int length)\n" << endl;
    return 0;
}

