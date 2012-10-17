classdef GravitySubscription < handle
    properties (Access = private)
        dataProductID;
        filter;
        subscriber;
    end
    
    methods (Access = public)
        function this = GravitySubscription(dataProductID, varargin)
            % Import the underlying Java subscriber class
            %import com.aphysci.gravity.matlab.MATLABGravitySubscriber;

            % Initialize the Subscriber
            this.subscriber = com.aphysci.gravity.matlab.MATLABGravitySubscriber;
            
            this.dataProductID = dataProductID;
            if (~isempty(varargin))
                this.filter = varargin{1};
            else
                this.filter = '';
            end
        end
        
        function gdp = getDataProduct(this, timeoutMS)
            gdp = this.subscriber.getDataProduct(timeoutMS);            
        end
        
        function dataProductID = getDataProductID(this)
            dataProductID = this.dataProductID();
        end
        
        function filter = getFilter(this)
            filter = this.filter;
        end
        
        function subscriber = getSubscriber(this)
            subscriber = this.subscriber;
        end
    end
end
