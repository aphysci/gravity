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

#ifndef __GRAVITYTEST_H
#define __GRAVITYTEST_H

#include <sstream>
#include <ostream>

#define GRAVITY_TEST_EQUALS(a,b) do { \
    if ( !( (a) == (b) ) ) \
    { \
        std::cerr << "Gravity assertion failed, " << a << " != " << b << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    } \
} while (0)

#define GRAVITY_TEST(a) do { \
    if ( !( a ) ) \
    { \
        std::cerr << "Gravity assertion failed at " << __FILE__ << ":" << __LINE__ << std::endl; \
        exit(1); \
    } \
} while (0)



#endif
