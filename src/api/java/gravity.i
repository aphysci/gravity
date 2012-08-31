
%include "enums.swg"
%javaconst(1);
%{
#include "GravityNode.h"
%}
%module gravity

namespace gravity {
    namespace GravityReturnCodes {
        enum Codes { 
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
    };

class GravityNode {
public:    
//    GravityReturnCode init();
};
};
