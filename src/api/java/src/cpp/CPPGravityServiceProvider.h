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


#ifndef CPPGRAVITYSERVICEPROVIDER_H_
#define CPPGRAVITYSERVICEPROVIDER_H_

#include "GravityServiceProvider.h"

namespace gravity
{

/**
 * Native implementation of a GravityServiceProvider
 */
class CPPGravityServiceProvider : public GravityServiceProvider
{
public:

    virtual ~CPPGravityServiceProvider();
    virtual std::tr1::shared_ptr<GravityDataProduct> request(const std::string serviceID, const GravityDataProduct& dataProduct);
    virtual std::tr1::shared_ptr<GravityDataProduct>  request(const std::string serviceID, char* array, int length);
};

}
#endif /* CPPGRAVITYSERVICEPROVIDER_H_ */

