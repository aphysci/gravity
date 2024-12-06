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
#include "spdlog/spdlog.h"

namespace gravity
{

class FileArchiver : public GravitySubscriber, GravityServiceProvider
{
private:
    static const char* ComponentName;
    GravityNode gravityNode;
    std::ofstream archiveFile;
    bool suspend;

    std::vector<std::string> split(std::string s);

    std::shared_ptr<spdlog::logger> logger;

public:
    FileArchiver();
    virtual ~FileArchiver();

    virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
    virtual std::shared_ptr<GravityDataProduct> request(const std::string serviceID,
                                                        const GravityDataProduct& dataProduct);
    void waitForExit();
};

} /* namespace gravity */
#endif /* FILEARCHIVER_H_ */
