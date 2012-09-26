
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
    virtual void requestFilled(string serviceID, string requestID, const GravityDataProduct& response);
    virtual int requestFilled(string serviceID, string requestID, char* array, int length);
};

}
#endif /* CPPGRAVITYREQUESTOR_H_ */

