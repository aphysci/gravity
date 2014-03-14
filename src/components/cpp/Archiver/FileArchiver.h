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

#ifndef FILEARCHIVER_H_
#define FILEARCHIVER_H_

#include "GravityNode.h"
#include <string>
#include <fstream>
#include <vector>

namespace gravity {

using namespace gravity;
using namespace std;

class FileArchiver : public GravitySubscriber
{
private:
	static const char* ComponentName;
	GravityNode gravityNode;
	ofstream archiveFile;

	vector<string> split(string s);
public:
	FileArchiver();
	virtual ~FileArchiver();

	virtual void subscriptionFilled(const vector<shared_ptr<GravityDataProduct> >& dataProducts);
	void waitForExit();
};

} /* namespace gravity */
#endif /* FILEARCHIVER_H_ */
