#include "GravityLogRecorder.h"

#include <string.h>
#include <stdio.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>

using namespace std;
using namespace std::tr1;

namespace gravity {

const string LogRecorder::logDataProductID("GRAVITY_LOGGER"); //Needs to be the same as gravity::Log::log_dataProductID.

//Note: Out filename is assumed to be 512 bytes long.
void LogRecorder::getNewFilename(char* outfilename, string basefilename)
{
    int len = filebasename.length();
    if(len > (511 - 19))
    {
        //throw new Exception("File Base name too long");
        cerr << "File Base name too long" << endl;
        strcpy(outfilename, "/dev/null");
    }

    //Add time string to filename.
    char* timestr = outfilename + len;
    time_t rawtime;
    struct tm * timeinfo;

    time(&rawtime);
    timeinfo = localtime( &rawtime );

    strftime(timestr, 100, "%Y_%m_%d_%H_%M_%S", timeinfo); //2012/08/23 12:12:12 - 19 chars

    strncpy(outfilename, filebasename.c_str(), len);
    strncpy(outfilename+len, timestr, 19);
}

LogRecorder::LogRecorder(GravityNode* gn, string filebasename)
{
    grav_node = gn;
    num_logs = 0;

    char filename[512];
    getNewFilename(filename, filebasename);

    log_file = fopen(filename, "w");
    if(log_file == NULL)
    {
        cerr << "Could not open Log file: " << filename << endl;
        //throw new Exception("");
    }
}

void LogRecorder::start()
{
    grav_node->subscribe(logDataProductID, *this);
}


void LogRecorder::subscriptionFilled(const std::vector< shared_ptr<GravityDataProduct> >& dataProducts)
{
    //shared_ptr<GravityDataProduct> gdp = *dataProducts.begin();
    //for_each(dataProducts.begin(), dataProducts.end(), [ this ] (shared_ptr<GravityDataProduct> dataProduct) { //Lambda function
    for(vector<shared_ptr<GravityDataProduct> >::const_iterator i = dataProducts.begin(); i != dataProducts.end(); i++)
    {
        shared_ptr<GravityDataProduct> dataProduct = *i;
        num_logs++;

        GravityLogMessagePB message;
        dataProduct->populateMessage(message);

        //Format the Logs nicely.
        char timestr[100];
        time_t rawtime = (time_t) (dataProduct->getGravityTimestamp() / 1000000);
        struct tm * timeinfo;

        timeinfo = gmtime( &rawtime );

        strftime(timestr, 100, "%m/%d/%y %H:%M:%S", timeinfo);

        fprintf(log_file, "[%s %s %s] %s\n", message.domain().c_str(), message.level().c_str(), timestr, message.message().c_str());
        fflush(log_file);

        if(num_logs >= NUM_LOGS_BEFORE_ROTATE)
            rotateLogs();
        return;
    }
}

void LogRecorder::rotateLogs()
{
    fclose(log_file);

    char filename[512];
    getNewFilename(filename, filebasename);

    log_file = fopen(filename, "w");
    if(log_file == NULL)
        cerr << "LogWriter::Rotate - Could not open Log file: " << filename << endl;
}

}
