#include "RustGravityHeartbeatListener.h"

namespace gravity
{
    void RustHeartbeatListener::MissedHeartbeat(std::string componentID,
                int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds)
    {
        this->missed(componentID, microsecond_to_last_heartbeat, interval_in_microseconds, this->addr);
    }
    void RustHeartbeatListener::ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds)
    {
        this->received(componentID, interval_in_microseconds, this->addr);
    }
    RustHeartbeatListener::RustHeartbeatListener(rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed,
            rust::Fn<void(const std::string&, int64_t&, size_t)> received,
            size_t addr)
    {
        this->received = received;
        this->missed = missed;
        this-> addr = addr;
    }

    std::unique_ptr<RustHeartbeatListener> rustNewHeartbeatListener(
            rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed,
            rust::Fn<void(const std::string&, int64_t&, size_t)> received,
            size_t addr
        ) 
    {
        return std::unique_ptr<RustHeartbeatListener>(new RustHeartbeatListener(missed, received, addr));
    }

    
} // namespace gravity

