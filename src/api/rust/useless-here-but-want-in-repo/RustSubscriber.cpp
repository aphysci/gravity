#include "RustSubscriber.h"

namespace gravity {
    RustSubscriber::RustSubscriber(RustThing rusty) 
    {
        r = rusty;
    }


    void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct>>& dataProducts)
    {
        
    }
                    

    } //namespace gravity