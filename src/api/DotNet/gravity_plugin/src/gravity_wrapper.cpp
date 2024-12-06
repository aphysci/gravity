#include <iostream>
#include <filesystem>
#include <sstream>
#include <GravityNode.h>
#include <GravityLogger.h>
#include <GravityRequestor.h>
#include <Utility.h>
#include "gravitywrapper_export.h"

#ifdef __cplusplus
extern "C"
{
#endif

    namespace GravityWrapper
    {

        struct gravityinterop_node
        {
            std::shared_ptr<gravity::GravityNode> node;
        };

        struct gravityinterop_dataproduct
        {
            std::shared_ptr<gravity::GravityDataProduct> dp;
        };

        typedef void (*gravityinterop_subscriptionFilled_cb)(gravityinterop_dataproduct**, int);
        typedef void (*gravityinterop_requestFilled_cb)(const char* serviceId, const char* requestId,
                                                        gravityinterop_dataproduct* dp);
        typedef gravityinterop_dataproduct* (*gravityinterop_servicerequest_cb)(const char* serviceId,
                                                                                gravityinterop_dataproduct* dp);
        typedef void (*gravityinterop_OnDebug_cb)(const char* message, int size);
        gravityinterop_OnDebug_cb g_gi_onDebugCb;
        std::mutex g_onDebugCbMutex;

        void LogError(const std::exception& ex, const char* name, const char* args)
        {
            std::lock_guard<std::mutex> lck(g_onDebugCbMutex);
            if (g_gi_onDebugCb)
            {
                std::stringstream ss;
                ss << ": GravityInterop: " << ((name) ? name : "") << ": " << ((args) ? args : "") << ": ";
                ss << ex.what();
                std::string str = ss.str();
                g_gi_onDebugCb(str.c_str(), str.length());
            }
        }

        void LogDebug(const char* name, const char* args)
        {
            std::lock_guard<std::mutex> lck(g_onDebugCbMutex);
            if (g_gi_onDebugCb)
            {
                std::stringstream ss;
                ss << ": GravityInterop: " << ((name) ? name : "") << ": " << ((args) ? args : "");
                std::string str = ss.str();
                g_gi_onDebugCb(str.c_str(), str.length());
            }
        }

        class SimpleGravityServiceProvider : public gravity::GravityServiceProvider
        {
        public:
            SimpleGravityServiceProvider(gravityinterop_servicerequest_cb onRequest) : m_onRequest(onRequest) {}

            virtual std::shared_ptr<gravity::GravityDataProduct> request(const std::string serviceID,
                                                                         const gravity::GravityDataProduct& dataProduct)
            {
                LogDebug("request", "Gravity request");

                std::lock_guard<std::mutex> lck(m_onRequestMutex);
                if (m_onRequest != nullptr)
                {
                    auto gi_dp = m_onRequest(
                        serviceID.c_str(),
                        new gravityinterop_dataproduct{std::make_shared<gravity::GravityDataProduct>(dataProduct)});
                    return gi_dp->dp;
                }
                return nullptr;
            }

            void setProviderCallback(gravityinterop_servicerequest_cb onRequest)
            {
                std::lock_guard<std::mutex> lck(m_onRequestMutex);
                m_onRequest = onRequest;
            }

        protected:
            gravityinterop_servicerequest_cb m_onRequest;
            std::mutex m_onRequestMutex;
        };

        class SimpleGravityRequestor : public gravity::GravityRequestor
        {
        public:
            SimpleGravityRequestor(gravityinterop_requestFilled_cb cb) : m_onFilled(cb) {}

            virtual void requestFilled(std::string serviceId, std::string requestId,
                                       const gravity::GravityDataProduct& gdp)
            {
                LogDebug("requestFilled", "Responded");

                std::lock_guard<std::mutex> lck(m_onFilledcbMutex);
                if (m_onFilled != nullptr)
                {
                    m_onFilled(serviceId.c_str(), requestId.c_str(),
                               new gravityinterop_dataproduct{std::make_shared<gravity::GravityDataProduct>(gdp)});
                }
            }

            void setRequestorCallback(gravityinterop_requestFilled_cb onFilled)
            {
                std::lock_guard<std::mutex> lck(m_onFilledcbMutex);
                m_onFilled = onFilled;
            }

        protected:
            gravityinterop_requestFilled_cb m_onFilled;
            std::mutex m_onFilledcbMutex;
        };

        class SimpleGravitySubscriber : public gravity::GravitySubscriber
        {
        public:
            SimpleGravitySubscriber(gravityinterop_subscriptionFilled_cb onFilled) : m_onFilled(onFilled) {}

            virtual void subscriptionFilled(
                const std::vector<std::shared_ptr<gravity::GravityDataProduct> >& dataProducts)
            {
                std::ostringstream oss;
                oss << "count: " << dataProducts.size();
                LogDebug("subscriptionFilled", oss.str().c_str());

                std::vector<gravityinterop_dataproduct*> gi_dps;
                std::lock_guard<std::mutex> lck(m_onFilledcbMutex);
                if (m_onFilled != nullptr)
                {
                    for (std::vector<std::shared_ptr<gravity::GravityDataProduct> >::const_iterator i =
                             dataProducts.begin();
                         i != dataProducts.end(); i++)
                    {
                        gi_dps.push_back(new gravityinterop_dataproduct{*i});
                    }
                    m_onFilled(&gi_dps[0], gi_dps.size());
                }
            }

            void setSubscriberCallback(gravityinterop_subscriptionFilled_cb onFilled)
            {
                std::lock_guard<std::mutex> lck(m_onFilledcbMutex);
                m_onFilled = onFilled;
            }

        protected:
            gravityinterop_subscriptionFilled_cb m_onFilled;
            std::mutex m_onFilledcbMutex;
        };

        struct gravityinterop_subscriber
        {
            std::shared_ptr<SimpleGravitySubscriber> subscriber;
        };

        struct gravityinterop_requestor
        {
            std::shared_ptr<SimpleGravityRequestor> requestor;
        };

        struct gravityinterop_serviceprovider
        {
            std::shared_ptr<SimpleGravityServiceProvider> provider;
        };

        GRAVITYWRAPPER_EXPORT gravityinterop_node* gravity_create_node(const char* componentId)
        {
            auto node = std::make_shared<gravity::GravityNode>();
            node->init(componentId);
            auto cwd = std::filesystem::current_path();
            LogDebug("GravityInit", cwd.string().c_str());

            return new gravityinterop_node{node};
        }

        GRAVITYWRAPPER_EXPORT void gravity_delete_node(gravityinterop_node* gi_node) { delete gi_node; }

        GRAVITYWRAPPER_EXPORT void gravity_register_serviceprovider(gravityinterop_node* gi_node, const char* serviceId,
                                                                    gravity::GravityTransportType type,
                                                                    gravityinterop_serviceprovider* sp)
        {
            gi_node->node->registerService(serviceId, type, *sp->provider);
        }

        GRAVITYWRAPPER_EXPORT gravityinterop_requestor* gravity_create_requestor()
        {
            auto requestor = std::make_shared<SimpleGravityRequestor>(nullptr);
            return new gravityinterop_requestor{requestor};
        }

        GRAVITYWRAPPER_EXPORT void gravity_set_requestor_callback(gravityinterop_requestor* gi_requestor,
                                                                  gravityinterop_requestFilled_cb cb)
        {
            gi_requestor->requestor->setRequestorCallback(cb);
        }

        GRAVITYWRAPPER_EXPORT void gravity_delete_requestor(gravityinterop_requestor* gi_req) { delete gi_req; }

        GRAVITYWRAPPER_EXPORT int gravity_request_async(gravityinterop_node* gi_node, const char* servName,
                                                        gravityinterop_dataproduct* gi_request_dp,
                                                        gravityinterop_requestor* gi_requestor, const char* requestId)
        {
            return gi_node->node->request(servName, *gi_request_dp->dp, *gi_requestor->requestor, requestId);
        }

        GRAVITYWRAPPER_EXPORT gravityinterop_dataproduct* gravity_request(gravityinterop_node* gi_node,
                                                                          const char* servName,
                                                                          gravityinterop_dataproduct* gi_request_dp,
                                                                          int timeoutMs)
        {
            return new gravityinterop_dataproduct{gi_node->node->request(servName, *gi_request_dp->dp, timeoutMs)};
        }

        GRAVITYWRAPPER_EXPORT gravityinterop_serviceprovider* gravity_create_serviceprovider()
        {
            auto provider = std::make_shared<SimpleGravityServiceProvider>(nullptr);
            return new gravityinterop_serviceprovider{provider};
        }

        GRAVITYWRAPPER_EXPORT void gravity_set_serviceprovider_callback(gravityinterop_serviceprovider* gi_sp,
                                                                        gravityinterop_servicerequest_cb cb)
        {
            gi_sp->provider->setProviderCallback(cb);
        }

        GRAVITYWRAPPER_EXPORT void gravity_delete_serviceprovider(gravityinterop_serviceprovider* gi_sp)
        {
            delete gi_sp;
        }

        GRAVITYWRAPPER_EXPORT gravityinterop_dataproduct* gravity_create_dataproduct(const char* dpId)
        {
            auto dp = std::make_shared<gravity::GravityDataProduct>(dpId);
            return new gravityinterop_dataproduct{dp};
        }

        GRAVITYWRAPPER_EXPORT void gravity_delete_dataproduct(gravityinterop_dataproduct* gi_dp) { delete gi_dp; }

        GRAVITYWRAPPER_EXPORT int gravity_getsize_dataproduct(gravityinterop_dataproduct* gi_dp)
        {
            return gi_dp->dp->getDataSize();
        }

        GRAVITYWRAPPER_EXPORT void gravity_getptr_dataproduct(gravityinterop_dataproduct* gi_dp, void* data, int size)
        {
            gi_dp->dp->getData(data, size);
        }

        GRAVITYWRAPPER_EXPORT void gravity_setdata_dataproduct(gravityinterop_dataproduct* gi_dp, void* data, int size)
        {
            gi_dp->dp->setData(data, size);
        }

        GRAVITYWRAPPER_EXPORT gravityinterop_subscriber* gravity_create_subscriber()
        {
            auto sub = std::make_shared<SimpleGravitySubscriber>(nullptr);
            return new gravityinterop_subscriber{sub};
        }

        GRAVITYWRAPPER_EXPORT void gravity_delete_subscriber(gravityinterop_subscriber* gi_sub) { delete gi_sub; }

        GRAVITYWRAPPER_EXPORT void gravity_set_subscriber_callback(gravityinterop_subscriber* gi_sub,
                                                                   gravityinterop_subscriptionFilled_cb on_filled)
        {
            gi_sub->subscriber->setSubscriberCallback(on_filled);
        }

        GRAVITYWRAPPER_EXPORT void gravity_node_publish(gravityinterop_node* gi_node, gravityinterop_dataproduct* gi_dp)
        {
            gi_node->node->publish(*gi_dp->dp);
            std::ostringstream oss;
            oss << "published dataproduct with id " << gi_dp->dp->getDataProductID();
            LogDebug("gravity_node_publish", oss.str().c_str());
        }

        GRAVITYWRAPPER_EXPORT void gravity_node_subscribe(const char* dpId, gravityinterop_node* gi_node,
                                                          gravityinterop_subscriber* gi_sub)
        {
            gi_node->node->subscribe(std::string(dpId), *gi_sub->subscriber);
            std::ostringstream oss;
            oss << "subscribed to dataproduct with id " << dpId;
            LogDebug("gravity_node_subscribe", oss.str().c_str());
        }

        GRAVITYWRAPPER_EXPORT void gravity_register_debug_callback(gravityinterop_OnDebug_cb onDebug)
        {
            std::lock_guard<std::mutex> lck(g_onDebugCbMutex);
            g_gi_onDebugCb = onDebug;
        }

        GRAVITYWRAPPER_EXPORT void gravity_register_dataproduct(gravityinterop_node* gi_node, const char* dpId,
                                                                gravity::GravityTransportType type)
        {
            gi_node->node->registerDataProduct(dpId, type);
        }
    }  // namespace GravityWrapper

#ifdef __cplusplus
}  //extern "C"
#endif