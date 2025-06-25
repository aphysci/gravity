#pragma once

#ifndef RUSTSUBSCRIBER_H_
#define RUSTSUBSCRIBER_H_

#include "GravityDataProduct.h"
#include "GravitySubscriber.h"

namespace gravity {
    class RustSubscriber : public GravitySubscriber {
        private: 
            RustThing r;
        public:
            RustSubscriber(RustThing rusty);
            void subscriptionFilled(
                const std::vector<GravityDataProduct>& dataProducts);
    };


}

#endif