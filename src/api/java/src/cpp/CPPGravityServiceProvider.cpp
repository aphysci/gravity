#include "CPPGravityServiceProvider.h"
#include <iostream>

using namespace gravity;

CPPGravityServiceProvider::~CPPGravityServiceProvider()
{}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(const std::string serviceID, const GravityDataProduct& dataProduct)
{
    unsigned char* array = new unsigned char[dataProduct.getSize()];
    dataProduct.serializeToArray(array);
    return request(serviceID, (char*)array, dataProduct.getSize());
	delete array;
}

shared_ptr<GravityDataProduct> CPPGravityServiceProvider::request(const std::string serviceID, char* array, int length)
{
    shared_ptr<GravityDataProduct> ret(new GravityDataProduct("CPP RESPONSE"));
    return ret;
}

