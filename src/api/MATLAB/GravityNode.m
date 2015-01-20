% (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
%
% Gravity is free software; you can redistribute it and/or modify
% it under the terms of the GNU Lesser General Public License as published by
% the Free Software Foundation; either version 3 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Lesser General Public License for more details.
%
% You should have received a copy of the GNU Lesser General Public
% License along with this program;
% If not, see <http://www.gnu.org/licenses/>.

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
        
        function ret = registerDataProduct(this, dataProductID, protocol)
            ret = this.gravityNode.registerDataProduct(dataProductID, protocol);
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
        
        function ret = registerService(this, serviceID, transportType, server)
            ret = this.gravityNode.registerService(serviceID, transportType, server.getJavaServer());
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
       
        function startHeartbeat(this, intervalMicroseconds)
            this.gravityNode.startHeartbeat(intervalMicroseconds);
        end
        
        function ret = getStringParam(this, key, varargin)
            if isempty(varargin)
                ret = this.gravityNode.getStringParam(key);
            else
                ret = this.gravityNode.getStringParam(key, varargin{1});
            end
            ret = char(ret);
        end
        
        function ret = getFloatParam(this, key, varargin)
            if isempty(varargin)
                ret = this.gravityNode.getFloatParam(key);
            else
                ret = this.gravityNode.getFloatParam(key, varargin{1});
            end
        end
        
        function ret = getIntParam(this, key, varargin)
            if isempty(varargin)
                ret = this.gravityNode.getIntParam(key);
            else
                ret = this.gravityNode.getIntParam(key, varargin{1});
            end
        end
    end
end
