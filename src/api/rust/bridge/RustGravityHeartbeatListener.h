#ifndef RUSTGRAVITYHEARTBEATLISTENER_H_
#define RUSTGRAVITYHEARTBEATLISTENER_H_

#include <memory>

#include <GravityHeartbeatListener.h>
#include "rust/cxx.h"


namespace gravity
{
    class RustHeartbeatListener : public GravityHeartbeatListener {
        private:
            rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed;
            rust::Fn<void(const std::string&, int64_t&, size_t)> received;
            size_t addr;
        public:
            RustHeartbeatListener(rust::Fn<void(
                const std::string&, int64_t, int64_t&, size_t)> missed,
                rust::Fn<void(const std::string&, int64_t&, size_t)> received,
                size_t addr
            );
            virtual void MissedHeartbeat(std::string componentID,
                int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds);
            virtual void ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds);

    };

    std::unique_ptr<RustHeartbeatListener> rustNewHeartbeatListener(
            rust::Fn<void(const std::string&, int64_t, int64_t&, size_t)> missed,
            rust::Fn<void(const std::string&, int64_t&, size_t)> received,
            size_t addr
        );

} // namespace gravity


#endif