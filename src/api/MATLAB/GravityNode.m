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
        
        initStatus;
    end
    
    methods (Access = public)
        function this = GravityNode(componentID)
            % Initialize the GravityNode
            this.gravityNode = com.aphysci.gravity.swig.GravityNode;

			if (isempty(componentID))
            	this.initStatus = this.gravityNode.init();
			else
            	this.initStatus = this.gravityNode.init(componentID);
			end
            
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

 			filter = '';
			domain = '';
			maxBufferSize = 0;
		    if (length(varargin) >= 1)
				filter = varargin{1};
			end
			if (length(varargin) >= 2)
				domain = varargin{2};
			end
			if (length(varargin) >= 3 && isnumeric(varargin{3}))
				maxBufferSize = floor(varargin{3});
			end

            % Create Subscription
            subscription = GravitySubscription(dataProductID, filter, domain, maxBufferSize);
            
            % Store subscriber
            key = [dataProductID ':' filter ':' domain];
            this.subscriptionMap(key) = subscription;
           
			if (length(varargin) >= 2)
                ret = this.gravityNode.subscribe(dataProductID, subscription.getSubscriber(), filter, domain);
            elseif (length(varargin) >= 1)
                ret = this.gravityNode.subscribe(dataProductID, subscription.getSubscriber(), filter);
			else
                ret = this.gravityNode.subscribe(dataProductID, subscription.getSubscriber());
			end	
        end
        
        function ret = unsubscribe(this, dataProductID, varargin)
            % Get Filter & Domain
 			filter = '';
			domain = '';
		    if (length(varargin) >= 1)
				filter = varargin{1};
			end
			if (length(varargin) >= 2)
				domain = varargin{2};
			end
            
            % Lookup subscriber
            key = [dataProductID ':' filter ':' domain];
            subscription = this.subscriptionMap(key);            
            
            if (~isempty(subscription))
				if (length(varargin) >= 2)
                	ret = this.gravityNode.subscribe(dataProductID, subscription.getSubscriber(), filter, domain);
                elseif (length(varargin) >= 1)
                    ret = this.gravityNode.unsubscribe(dataProductID, subscription.getSubscriber(), filter);
				else
                	ret = this.gravityNode.subscribe(dataProductID, subscription.getSubscriber());
				end	
            end
        end               
        
        function ret = registerService(this, serviceID, transportType, server)
            ret = this.gravityNode.registerService(serviceID, transportType, server.getJavaServer());
        end
        
        function ret = unregisterService(this, serviceID)
            ret = this.gravityNode.unregisterService(serviceID);
        end
        
        function ret = request(this, serviceID, gravityDataProduct, requestor, requestID, timeout, varargin)            
            if ~isempty(varargin)
                ret = this.gravityNode.request(serviceID, gravityDataProduct, requestor.getJavaRequestor(), ...
												requestID, timeout, varargin{1});
            else
                ret = this.gravityNode.request(serviceID, gravityDataProduct, requestor.getJavaRequestor(),...
												requestID, timeout);
            end
        end
      
	   	function stopHeartbeat(this)
			this.gravityNode.stopHeartbeat();
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

		function ret = getIP(this)
			ret = this.gravityNode.getIP();
		end
		
		function ret = getDomain(this)
			ret = this.gravityNode.getDomain();
        end
        
        function ret = getInitStatus(this)
            ret = this.initStatus;
        end
    end
end
