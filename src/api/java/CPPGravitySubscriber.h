
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

    void subscriptionFilled(const GravityDataProduct& dataProduct);
};

}
#endif /* CPPGRAVITYSUBSCRIBER_H_ */

