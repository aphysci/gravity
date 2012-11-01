

#include "CPPGravityRequestor.h"
#include <iostream>

using namespace gravity;

CPPGravityRequestor::~CPPGravityRequestor()
{}

void CPPGravityRequestor::requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
{
    unsigned char* array = new unsigned char[response.getSize()];
    response.serializeToArray(array);
    requestFilled(serviceID, requestID, (char*)array, response.getSize());
	delete array;
}

char CPPGravityRequestor::requestFilled(const std::string& serviceID, const std::string& requestID, char* array, int length)
{
    return 0;
}

