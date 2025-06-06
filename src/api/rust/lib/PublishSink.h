/** (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
 **
 ** Gravity is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU Lesser General Public License as published by
 ** the Free Software Foundation; either version 3 of the License, or
 ** (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Lesser General Public License for more details.
 **
 ** You should have received a copy of the GNU Lesser General Public
 ** License along with this program;
 ** If not, see <http://www.gnu.org/licenses/>.
 **
 */

#include "spdlog/sinks/base_sink.h"
#include "GravityNode.h"
#include "GravityDataProduct.h"
#include "protobuf/GravityLogMessagePB.pb.h"
#include "CommUtil.h"

#include <iostream>

namespace gravity
{

template <typename Mutex>
class proxy_dist_sink : public spdlog::sinks::dist_sink<Mutex>
{
    using BaseSink = spdlog::sinks::dist_sink<Mutex>;

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        if (BaseSink::should_log(msg.level))
        {
            BaseSink::sink_it_(msg);
        }
    }
};

template <typename Mutex>
class PublishSink : public spdlog::sinks::base_sink<Mutex>
{
private:
    GravityNode* gn;
    std::mutex lock;
    bool init;

    bool checkInit()
    {
        std::lock_guard<std::mutex> guard(lock);
        if (!init)
        {
            //GravityReturnCode ret  = gn->registerDataProduct(gravity::constants::GRAVITY_LOGGER_DPID, GravityTransportTypes::TCP);
            GravityReturnCode ret = gn->registerDataProductInternal(
                gravity::constants::GRAVITY_LOGGER_DPID, GravityTransportTypes::TCP, false, false, false, true);
            init = ret == GravityReturnCodes::SUCCESS;
        }
        return init;
    }

public:
    PublishSink(GravityNode* gn) : init(false) { this->gn = gn; }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        if (checkInit())
        {
            // Format message
            spdlog::memory_buf_t formatted;
            spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

            GravityDataProduct dp(gravity::constants::GRAVITY_LOGGER_DPID);
            gravity::GravityLogMessagePB logMessage;
            logMessage.set_level(spdlog::level::to_string_view(msg.level).data());
            logMessage.set_message(fmt::to_string(formatted));
            dp.setData(logMessage);
            gn->publish(dp);
        }
    }

    void flush_() override
    {
        // Nothing to do here
    }
};
}  // namespace gravity
