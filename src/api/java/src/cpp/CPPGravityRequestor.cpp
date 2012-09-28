

#include "CPPGravityRequestor.h"
#include <iostream>

using namespace gravity;

CPPGravityRequestor::~CPPGravityRequestor()
{}

void CPPGravityRequestor::requestFilled(string serviceID, string requestID, const GravityDataProduct& response)
{
    cout << "made it into CPPGravityRequestor::requestFilled(string serviceID, string requestID, const GravityDataProduct& dataProduct), response id = "
         << response.getDataProductID() << endl;
    unsigned char array[response.getSize()];
    response.serializeToArray(array);
    requestFilled(serviceID, requestID, (char*)array, response.getSize());
}

int CPPGravityRequestor::requestFilled(const std::string& serviceID, const std::string& requestID, char* array, int length)
{
    cout << "made it into CPPGravityRequestor::requestFilled(string serviceID, string requestID, const signed char* array, int length)\n" << endl;
    return 0;
}

