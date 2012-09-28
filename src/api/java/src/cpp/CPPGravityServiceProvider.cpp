

#include "CPPGravityServiceProvider.h"
#include <iostream>

using namespace gravity;

CPPGravityServiceProvider::~CPPGravityServiceProvider()
{}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(const GravityDataProduct& dataProduct)
{
    unsigned char array[dataProduct.getSize()];
    dataProduct.serializeToArray(array);
    return request((char*)array, dataProduct.getSize());
}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(char* array, int length)
{
    shared_ptr<GravityDataProduct> ret(new GravityDataProduct("CPP RESPONSE"));
    return ret;
}

