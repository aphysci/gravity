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
            this.gravityDataProduct.setData(protobuf.getProtobufBuilder().build());
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
    end
end
