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

classdef GravityDataProduct < handle
    properties (Access = private)
        gravityDataProduct;
    end
    
    methods (Access = public)
        function this = GravityDataProduct(dataProductID)
            this.gravityDataProduct = com.aphysci.gravity.GravityDataProduct(dataProductID);
        end               
        
        function timestamp = getGravityTimestamp(this)
            timestamp = this.gravityDataProduct.getGravityTimestamp();
        end
        
        function receivedTimestamp = getReceivedTimestamp(this)
            receivedTimestamp = this.gravityDataProduct.getReceivedTimestamp();
        end

        function dataProductID = getDataProductID(this)
            dataProductID = this.gravityDataProduct.getDataProductID();
        end
        
        function softwareVersion = getSoftwareVersion(this)
            softwareVersion = this.gravityDataProduct.getSoftwareVersion();
        end
        
        function setSoftwareVersion(softwareVersion)
            this.gravityDataProduct.setSoftwareVersion(softwareVersion);
        end
        
        function setData(this, protobuf)
            this.gravityDataProduct.setData(protobuf.getProtobufBuilder());
        end
        
        function populateMessage(this, protobuf)
            this.gravityDataProduct.populateMessage(protobuf.getProtobufBuilder());
        end
        
        function setGravityDataProduct(this, gdp)
            this.gravityDataProduct = gdp;
        end
        
        function gdp = getGravityDataProduct(this)
            gdp = this.gravityDataProduct;
        end

		function domain = getDomain(this)
			domain = this.gravityDataProduct.getDomain();
		end

		function componentID = getComponentID(this)
			componentID = this.gravityDataProduct.getComponentID();
		end

		function setTimestamp(this, ts)
			this.gravityDataProduct.setTimestamp(ts);
	    end
		
		function setDomain(this, domain)
			this.gravityDataProduct.setDomain(domain);
	    end
		
		function setComponentID(this, componentID)
			this.gravityDataProduct.setComponentID(componentID);
	    end
    end
end
