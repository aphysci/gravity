#ifndef RUSTGRAVITYHEARTBEATLISTENER_H_
#define RUSTGRAVITYHEARTBEATLISTENER_H_

#include <memory>

#include <GravityHeartbeatListener.h>
#include "rust/cxx.h"

struct ListenerWrap;

namespace gravity
{
    class RustHeartbeatListener : public GravityHeartbeatListener {
        private:
            rust::Fn<void(const std::string&, int64_t, int64_t&, ListenerWrap *)> missed;
            rust::Fn<void(const std::string&, int64_t&, ListenerWrap *)> received;
            ListenerWrap * listenerPtr;
        public:
            RustHeartbeatListener(rust::Fn<void(
                const std::string&, int64_t, int64_t&, ListenerWrap *)> missed,
                rust::Fn<void(const std::string&, int64_t&, ListenerWrap *)> received,
                ListenerWrap * listenerPtr
            );
            virtual void MissedHeartbeat(std::string componentID,
                int64_t microsecond_to_last_heartbeat, int64_t& interval_in_microseconds);
            virtual void ReceivedHeartbeat(std::string componentID, int64_t& interval_in_microseconds);

    };

    std::unique_ptr<RustHeartbeatListener> rustNewHeartbeatListener(
            rust::Fn<void(const std::string&, int64_t, int64_t&, ListenerWrap *)> missed,
            rust::Fn<void(const std::string&, int64_t&, ListenerWrap *)> received,
            ListenerWrap * listenerPtr
        );

} // namespace gravity


#endif