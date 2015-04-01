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

classdef ServiceDirectoryDomainUpdate < Protobuf
    properties (Access = private)
        builder;
    end
    
    methods (Access = public)
        function this = ServiceDirectoryDomainUpdate()
            this.builder = javaMethod('newBuilder', 'com.aphysci.gravity.protobuf.ServiceDirectoryDomainUpdateContainer$ServiceDirectoryDomainUpdatePB');
        end
        
        function builder = getProtobufBuilder(this)
            builder = this.builder;
        end
        
        function domainList = getKnownDomainsList(this)
           domainList = {};
           for i = 1 : this.builder.getKnownDomainsCount()
               domainList{i} = char(this.builder.getKnownDomains(i-1));
           end
        end
        
        function domainsCount = getKnownDomainsCount(this)
            domainsCount = this.builder.getKnownDomainsCount();
        end
        
        function domain = getKnownDomains(this, index)
            domain = char(this.builder.getKnownDomain(index-1));
        end
        
        function updateDomain = getUpdateDomain(this)
            updateDomain = char(this.builder.getUpdateDomain());
        end
        
        function add = isAdd(this)
            add = strcmp(this.builder.getType(), 'ADD');
        end
        
        function remove = isRemove(this)
            remove = strcmp(this.builder.getType(), 'REMOVE');
        end
    end
    
end