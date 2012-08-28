#include "GravityLogger.h"
#include <string>
#include <map>

struct _dictionary_;
typedef struct _dictionary_ dictionary ;

namespace gravity {

class GravityConfigParser
{
public:
    GravityConfigParser();
    void ParseCmdLine(int argc, const char* argv);
    void ParseConfigFile(const char* filename, const char** additional_keys_to_extract = NULL);

    std::string getServiceDirectoryUrl() { return serviceDirectoryUrl; }
    gravity::Log::LogLevel getLocalLogLevel() { return log_local_level; }
    gravity::Log::LogLevel getNetLogLevel() { return log_net_level; }

    std::string getValue(const char* key) { return values[key]; }

    /*
     * Virtual Destructor: for overriding.
     */
    virtual ~GravityConfigParser() { }
protected:
    /*
     * Override this function to handle custom processing of the ini file.
     */
    virtual void GetOtherConfigOptions(dictionary* myconfig);
private:
    char* serviceDirectoryUrl;
    gravity::Log::LogLevel log_local_level;
    gravity::Log::LogLevel log_net_level;

    std::map<std::string, std::string> values;
};

}
