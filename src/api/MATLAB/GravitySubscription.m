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
            gdp = [];
            jgdp = this.subscriber.getDataProduct(timeoutMS);
            if (~isempty(jgdp))
                gdp = GravityDataProduct(jgdp.getDataProductID());            
                gdp.setGravityDataProduct(jgdp);
            end
        end
        
        function gdps = getAllDataProducts(this)
            dps = this.subscriber.getAllDataProducts();
            gdps = cell(dps.size(),1);
            for i = 1 : dps.size()
                gdps{i} = GravityDataProduct(dps.get(i-1).getDataProductID());
                gdps{i}.setGravityDataProduct(dps.get(i-1));
            end
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
