#include <MOOS/libMOOS/Comms/MOOSAsyncCommClient.h>

#include <cstdio>
#include <unistd.h>

using namespace std;

bool onConnect(void *param);

class MOOSPublisher {
public:
    MOOS::MOOSAsyncCommClient _comms;
    bool connected;

    MOOSPublisher() : connected(false) {   
        _comms.SetOnConnectCallBack(&onConnect, this);
        _comms.Run("localhost", 9000, "MOOSPublisher", 10);
    }
    
    virtual ~MOOSPublisher() { }

    void transmit() {
        static unsigned counter = 0;

        char mymessage[100];
        snprintf(mymessage, 99, "Test: %d", counter++);
        _comms.Notify("TEST_PUBLICATION", std::string(mymessage));
    }
};


bool onConnect(void *param) {
    static_cast<MOOSPublisher*>(param)->connected = true;
    return true;
}

int main(int argc, char *argv[]) {
    MOOSPublisher pub;
    while (true) {
        if (pub.connected) {
            pub.transmit();
        }
        usleep(500000);
    }
}
