#include "GravityNode.h"
#include "GravitySubscriber.h"
#include "GravityLogMessagePB.pb.h"
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
    virtual void subscriptionFilled(string dataProductID, const vector<shared_ptr<GravityDataProduct> > dataProducts);
private:
    GravityNode* grav_node;
    FILE* log_file;
    std::string filebasename;
    int num_logs;

    static const string logDataProductID;

    void getNewFilename(char* outfilename, string basefilename);

    void rotateLogs();
};

}
