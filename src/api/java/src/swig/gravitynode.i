
%typemap(javaimports) gravity::GravityNode %{
import com.aphysci.gravity.GravityDataProduct;
import com.aphysci.gravity.GravitySubscriber;
import com.aphysci.gravity.GravityRequestor;
import com.aphysci.gravity.GravityServiceProvider;
import com.aphysci.gravity.GravityHeartbeatListener;
%}

// this turns on director features for CPPGravitySubscriber
%feature("director") gravity::CPPGravitySubscriber;
%feature("director") gravity::CPPGravityRequestor;
%feature("director") gravity::CPPGravityServiceProvider;
%feature("director") gravity::CPPGravityHeartbeatListener;

// This is where we actually declare the types and methods that will be made available in Java.  This section must be kept in
// sync with the Gravity API.
namespace gravity {

	class CPPGravitySubscriber {
	public:
		virtual ~CPPGravitySubscriber();
		virtual int subscriptionFilled(char *BYTE, int byteLength, int *INTEGER, int intLength);
	};

	class CPPGravityRequestor {
	public:
		virtual ~CPPGravityRequestor();
		virtual char requestFilled(const std::string& serviceID, const std::string& requestID, char *BYTE, int byteLength);
	};

	class CPPGravityServiceProvider {
	public:
		virtual ~CPPGravityServiceProvider();
		virtual shared_ptr<gravity::GravityDataProduct> request(char *BYTE, int byteLength);
	};

	class CPPGravityHeartbeatListener
	{
	public:
	    virtual ~CPPGravityHeartbeatListener();
    	virtual void MissedHeartbeat(const std::string& dataProductID, int microsecond_to_last_heartbeat, const std::string& status);
    	virtual void ReceivedHeartbeat(const std::string& dataProductID, const std::string& status);
	};

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
        INTERRUPTED = -10,
        NO_SERVICE_PROVIDER = -11,
        NO_PORTS_AVAILABLE = -12
    };

class GravityNode {
public:
	GravityNode();
	~GravityNode();
    GravityReturnCode init(std::string);
    void waitForExit();
    GravityReturnCode registerDataProduct(const std::string& dataProductID, const std::string &transportType);
    GravityReturnCode unregisterDataProduct(const std::string& dataProductID);
    
    GravityReturnCode subscribe(const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, const std::string& filter = "");
    GravityReturnCode unsubscribe(const std::string& dataProductID, const gravity::GravitySubscriber& subscriber, const std::string& filter = "");
    
	GravityReturnCode publish(const gravity::GravityDataProduct& dataProduct, const std::string& filter = "");
	
	GravityReturnCode request(const std::string& serviceID, const gravity::GravityDataProduct& dataProduct, 
	        const gravity::GravityRequestor& requestor, const std::string& requestID = "");
    shared_ptr<gravity::GravityDataProduct> request(const std::string& serviceID, const gravity::GravityDataProduct& request, int timeout_milliseconds = -1);
            
    GravityReturnCode registerService(const std::string& serviceID, const std::string& transportType, 
    		const gravity::GravityServiceProvider& server);
    GravityReturnCode unregisterService(const std::string& serviceID);
    
    GravityReturnCode startHeartbeat(int interval_in_microseconds);
    GravityReturnCode registerHeartbeatListener(const std::string& dataProductID, unsigned long timebetweenMessages, const gravity::GravityHeartbeatListener& listener);

    std::string getStringParam(std::string key, std::string default_value = "");
    int getIntParam(std::string key, int default_value = -1);
    double getFloatParam(std::string key, double default_value = 0.0);
    bool getBoolParam(std::string key, bool default_value = false);

};

};

