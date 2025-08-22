#include "RustGravityHeartbeatListener.h"
#include "gravity/src/api/rust/src/ffi.rs.h"

namespace gravity
{
    void RustHeartbeatListener::MissedHeartbeat(std::string componentID,
                int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds)
    {
        this->missed(componentID, microsecond_to_last_heartbeat, interval_in_microseconds, this->listenerPtr);
    }
    void RustHeartbeatListener::ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds)
    {
        this->received(componentID, interval_in_microseconds, this->listenerPtr);
    }
    RustHeartbeatListener::RustHeartbeatListener(rust::Fn<void(const std::string&, int64_t, int64_t&, ListenerWrap *)> missed,
            rust::Fn<void(const std::string&, int64_t&, ListenerWrap *)> received,
            ListenerWrap * listenerPtr) : listenerPtr(listenerPtr)
    {
        this->received = received;
        this->missed = missed;
    }

    std::unique_ptr<RustHeartbeatListener> rustNewHeartbeatListener(
            rust::Fn<void(const std::string&, int64_t, int64_t&, ListenerWrap *)> missed,
            rust::Fn<void(const std::string&, int64_t&, ListenerWrap *)> received,
            ListenerWrap * listenerPtr
        ) 
    {
        return std::unique_ptr<RustHeartbeatListener>(new RustHeartbeatListener(missed, received, listenerPtr));
    }

    
} // namespace gravity

