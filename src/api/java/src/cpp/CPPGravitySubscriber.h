/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

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
    virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
    virtual int subscriptionFilled(char* array, int arrayLength, int* lengths, int length);
};

}  // namespace gravity
#endif /* CPPGRAVITYSUBSCRIBER_H_ */
