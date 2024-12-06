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

#include "DomainDataKey.h"

DomainDataKey::DomainDataKey(std::string domain, std::string dataProductID)
{
    this->domain = domain;
    this->dataProductID = dataProductID;
}

DomainDataKey::~DomainDataKey(void) {}

bool DomainDataKey::operator<(const DomainDataKey &right) const
{
    bool ret = false;
    if (domain == right.domain)
    {
        ret = dataProductID < right.dataProductID;
    }
    else
    {
        ret = domain < right.domain;
    }

    return ret;
}

bool DomainDataKey::operator>(const DomainDataKey &right) const { return (!(*this < right || *this == right)); }

bool DomainDataKey::operator==(const DomainDataKey &right) const { return (!(*this < right) && !(right < *this)); }

bool DomainDataKey::operator!=(const DomainDataKey &right) const { return (!(*this == right)); }
