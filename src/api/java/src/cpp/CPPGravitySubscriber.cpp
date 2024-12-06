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

#include "CPPGravitySubscriber.h"
#include <iostream>
#include <memory>

using namespace gravity;

CPPGravitySubscriber::~CPPGravitySubscriber() {}

void CPPGravitySubscriber::subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts)
{
    int* lengths = new int[dataProducts.size()];
    int arrayLength = 0;
    for (unsigned int index = 0; index < dataProducts.size(); index++)
    {
        arrayLength += dataProducts[index]->getSize();
        lengths[index] = dataProducts[index]->getSize();
    }
    unsigned char* array = new unsigned char[arrayLength];
    int offset = 0;
    for (unsigned int index = 0; index < dataProducts.size(); index++)
    {
        dataProducts[index]->serializeToArray(&array[offset]);
        offset += lengths[index];
    }
    subscriptionFilled((char*)array, arrayLength, lengths, dataProducts.size());
    delete[] lengths;
    delete[] array;
}

int CPPGravitySubscriber::subscriptionFilled(char* array, int arrayLength, int* lengths, int length) { return 0; }
