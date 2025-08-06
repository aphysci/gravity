#ifndef RUSTSUBSCRIBER_H_
#define RUSTSUBSCRIBER_H_

#include <GravitySubscriber.h>
#include "rust/cxx.h"

struct SubscriberWrap;

namespace gravity
{
   

    class RustSubscriber : public GravitySubscriber {
        private:
            rust::Fn<void(const std::vector<GravityDataProduct >&, const SubscriberWrap *)> func;
            size_t addr;
            const SubscriberWrap * box;
        public:
            RustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, const SubscriberWrap*)> func, size_t addr, const SubscriberWrap*);
            virtual void subscriptionFilled(const std::vector<std::shared_ptr<GravityDataProduct> >& dataProducts);
    };

    
    std::unique_ptr<RustSubscriber> newRustSubscriber(rust::Fn<void(const std::vector<GravityDataProduct >&, const SubscriberWrap*)> func, size_t addr, const SubscriberWrap* box);
    
} // namespace gravity


#endif