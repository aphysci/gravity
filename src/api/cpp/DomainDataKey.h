#ifndef DOMAINDATAKEY_H_
#define DOMAINDATAKEY_H_

#include <string>

class DomainDataKey
{
private:
	std::string domain;
	std::string dataProductID;

public:
	DomainDataKey(std::string domain, std::string dataProductID);	
	~DomainDataKey(void);

	bool operator<(const DomainDataKey &right) const;
	bool operator==(const DomainDataKey &right) const;
};

#endif /* DOMAINDATAKEY_H_ */

