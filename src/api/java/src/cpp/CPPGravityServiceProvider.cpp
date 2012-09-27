

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
    GravityDataProduct gdp("");
    return request((char*)array, dataProduct.getSize(), gdp);
//    shared_ptr<GravityDataProduct> ret(new GravityDataProduct("RESPONSE"));
//    return ret;
}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(char* array, int length, GravityDataProduct& outputResponse)
{
    cout << "made it into CPPGravityServiceProvider::request(const signed char* array, int length)\n" << endl;
    shared_ptr<GravityDataProduct> ret(new GravityDataProduct("RESPONSE"));
    return ret;
//    return 0;
}

