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

#ifndef FILEREPLAY_H_
#define FILEREPLAY_H_

#include "GravityNode.h"
#include <fstream>
#include <set>
#include "spdlog/spdlog.h"

namespace gravity
{

using namespace gravity;
using namespace std;

class FileReplay
{
private:
    static const char* ComponentName;
    uint64_t firstPublishTime;
    uint64_t firstDataTime;

    FileReader fileReader;

    GravityNode gravityNode;
    set<string> datatypes;

    void processArchive();

    std::shared_ptr<spdlog::logger> logger;

public:
    FileReplay();
    virtual ~FileReplay();
    void waitForExit();
};

} /* namespace gravity */
#endif /* FILEREPLAY_H_ */
