
#ifndef CPPGRAVITYSUBSCRIBER_H_
#define CPPGRAVITYSUBSCRIBER_H_

#include "GravitySubscriber.h"

namespace gravity
{

/**
 * Native implementation of a GravitySubscriber
 */
class CPPGravitySubscriber : public GravitySubscriber
{
public:

    virtual ~CPPGravitySubscriber();
    virtual void subscriptionFilled(const GravityDataProduct& dataProduct);
    virtual int subscriptionFilled(char* array, int length);
};

}
#endif /* CPPGRAVITYSUBSCRIBER_H_ */

