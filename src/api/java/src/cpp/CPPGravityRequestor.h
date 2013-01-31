
#ifndef CPPGRAVITYREQUESTOR_H_
#define CPPGRAVITYREQUESTOR_H_

#include "GravityRequestor.h"

namespace gravity
{

/**
 * Native implementation of a GravityRequestor
 */
class CPPGravityRequestor : public GravityRequestor
{
public:

    virtual ~CPPGravityRequestor();
    virtual void requestFilled(std::string serviceID, std::string requestID, const GravityDataProduct& response);
    virtual char requestFilled(const std::string& serviceID, const std::string& requestID, char* array, int length);
};

}
#endif /* CPPGRAVITYREQUESTOR_H_ */

