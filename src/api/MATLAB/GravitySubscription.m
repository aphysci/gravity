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
