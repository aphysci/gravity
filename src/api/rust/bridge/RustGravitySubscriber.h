#ifndef RUSTSUBSCRIBER_H_
#define RUSTSUBSCRIBER_H_

#include <GravitySubscriber.h>
#include "rust/cxx.h"


namespace gravity
{
    class RustSubscriber : public GravitySubscriber {
        private:
            rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func;
            size_t addr;
        public:
            RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func, size_t addr);
            virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
    };

    
    std::unique_ptr<RustSubscriber> newRustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, size_t)> func, size_t addr);
    
} // namespace gravity


#endif