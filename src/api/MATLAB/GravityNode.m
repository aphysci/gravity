classdef GravityNode < handle
    properties (Access = private)
        gravityNode;
        
        subscriptionMap;
    end
    
    methods (Access = public)
        function this = GravityNode(componentID)
            % Import the underlying Java classes
            %import com.aphysci.gravity.swig.GravityNode;            

            % Initialize the GravityNode
            this.gravityNode = com.aphysci.gravity.swig.GravityNode;
            this.gravityNode.init(componentID);
            
            % Initialize the subscription map
            this.subscriptionMap = containers.Map;
        end
        
        function ret = registerDataProduct(this, dataProductID, networkPort, protocol)
            ret = this.gravityNode.registerDataProduct(dataProductID, networkPort, protocol);
        end
        
        function ret = unregisterDataProduct(this, dataProductID)
            ret = this.gravityNode.unregisterDataProduct(dataProductID);
        end
        
        function ret = publish(this, dataProduct)
            ret = this.gravityNode.publish(dataProduct.getGravityDataProduct());
        end
        
        function subscription = subscribe(this, dataProductID, varargin)
            %import com.aphysci.gravity.matlab.MATLABGravitySubscriber;
            
            % Get Filter
            if (isempty(varargin))
                filter = '';
            else
                filter = varargin{1};
            end                        
            
            % Create Subscription
            subscription = GravitySubscription(dataProductID, filter);
            
            % Store subscriber
            key = [dataProductID ':' filter];
            this.subscriptionMap(key) = subscription;
            
            if ~isempty(varargin)
                ret = this.gravityNode.subscribe(dataProductID, subscription.getSubscriber(), filter);
            else
                ret = this.gravityNode.subscribe(dataProductID, subscription.getSubscriber());
            end
        end
        
        function ret = unsubscribe(this, dataProductID, varargin)
            % Get Filter
            if (isempty(varargin))
                filter = '';
            else
                filter = varargin{1};
            end 
            
            % Lookup subscriber
            key = [dataProductID ':' filter];
            subscription = this.subscriptionMap(key);            
            
            if (~isempty(subscription))
                if ~isempty(varargin)
                    ret = this.gravityNode.unsubscribe(dataProductID, subscription.getSubscriber(), filter);
                else
                    ret = this.gravityNode.unsubscribe(dataProductID, subscription.getSubscriber());
                end
            end
        end               
        
        function ret = registerService(this, serviceID, networkPort, transportType, server)
            ret = this.gravityNode.registerService(serviceID, networkPort, transportType, server.getJavaServer());
        end
        
        function ret = unregisterService(this, serviceID)
            ret = this.gravityNode.unregisterService(serviceID);
        end
        
        function ret = request(this, serviceID, gravityDataProduct, requestor, varargin)            
            if ~isempty(varargin)
                ret = this.gravityNode.request(serviceID, gravityDataProduct, requestor.getJavaRequestor(), varargin{1});
            else
                ret = this.gravityNode.request(serviceID, gravityDataProduct, requestor.getJavaRequestor());
            end
        end
        
        function startHeartbeat(this, componentID, intervalMicroseconds, varargin)
            if ~isempty(varargin)
                this.gravityNode.startHeartbeat(componentID, intervalMicroseconds, varargin{1});
            else
                this.gravityNode.startHeartbeat(componentID, intervalMicroseconds, 54541);            
            end
        end
    end
end
