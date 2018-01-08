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

#include "GravityNode.h"
#include "GravitySubscriber.h"
#include "protobuf/GravityLogMessagePB.pb.h"
#include <string>

namespace gravity {

#define NUM_LOGS_BEFORE_ROTATE (1000000/81) //About 1MB.  (1Mil chars / 81 (chars/line))

class LogRecorder : public GravitySubscriber
{
public:
    /*
     * Initalizes the Log Writer
     * \param grav_node Assumed to be initialized.
     * \param filebasename The base of the file name, to be appended with the start date.  Can include the directory.
     */
    LogRecorder(GravityNode* grav_node, std::string filebasename);
    /*
     * Starts the Logger (Calls Subscribe).
     */
    void start();

    /**
     * Writes to the Log.
     */
    virtual void subscriptionFilled(const std::vector< std::tr1::shared_ptr<GravityDataProduct> >& dataProducts);
private:
    GravityNode* grav_node;
    FILE* log_file;
    std::string filebasename;
    int num_logs;

    static const std::string logDataProductID;

    void getNewFilename(char* outfilename, std::string basefilename);

    void rotateLogs();
};

}
