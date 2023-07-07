#include <iostream>
#include <GravitySubscriber.h>
#include <Utility.h>

#include "protobuf/GravitySpdLogConfigPB.pb.h"


namespace gravity {
    //Declare class for receiving published messages about the log.
    class SpdLogConfigSubscriber: public GravitySubscriber
    {
        private:
            string componentID;
            void reconfigSpdLoggers(GravitySpdLogConfigPB spdLogConfigPB);
        public:
            void subscriptionFilled(const std::vector< std::shared_ptr<GravityDataProduct> >& dataProducts);
    };
}