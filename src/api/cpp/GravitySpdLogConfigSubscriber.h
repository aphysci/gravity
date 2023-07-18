#include <iostream>
#include "Utility.h"
#include "GravitySubscriber.h"
#include "protobuf/GravitySpdLogConfigPB.pb.h"

namespace gravity {
    //Declare class for receiving published messages about the log.
    class SpdLogConfigSubscriber: public GravitySubscriber
    {
        private:
            std::string componentID;
            void reconfigSpdLoggers(GravitySpdLogConfigPB spdLogConfigPB);
        public:
            SpdLogConfigSubscriber();
            void init(std::string compID);
            void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts);

    };
}