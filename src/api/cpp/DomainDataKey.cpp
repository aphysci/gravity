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

bool DomainDataKey::operator==(const DomainDataKey &right) const
{
	return ( !(*this < right) && !(right < *this) );
}
