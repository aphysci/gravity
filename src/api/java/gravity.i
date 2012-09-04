
%include "enums.swg"
%include "std_string.i" // for std::string typemaps

%javaconst(1);
%{
#include "GravityNode.h"
%}
%module gravity

namespace gravity {
    enum GravityReturnCode {
        SUCCESS = 0,
        FAILURE = -1,
        NO_SERVICE_DIRECTORY = -2,
        REQUEST_TIMEOUT = -3,
        DUPLICATE = -4,
        REGISTRATION_CONFLICT = -5,
        NOT_REGISTERED = -6,
        NO_SUCH_SERVICE = -7,
        NO_SUCH_DATA_PRODUCT = -8,
        LINK_ERROR = -9,
        INTERRUPTED = -10
    };

class GravityNode {
public:    
    GravityReturnCode init();
    GravityReturnCode registerDataProduct(const std::string& dataProductID, unsigned short networkPort, const std::string &transportType);
    GravityReturnCode unregisterDataProduct(const std::string& dataProductID);
//    GravityReturnCode subscribe(const std::string& dataProductID, const GravitySubscriber& subscriber, const std::string& filter = "");

};
};
