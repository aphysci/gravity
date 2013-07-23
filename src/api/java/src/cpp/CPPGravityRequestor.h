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

