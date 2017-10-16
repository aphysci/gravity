# (C) Copyright 2013, Applied Physical Sciences Corp., A General Dynamics Company
#
# Gravity is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this program;
# If not, see <http://www.gnu.org/licenses/>.
#
 
import traceback

from GravityDataProductPB_pb2 import GravityDataProductPB

class GravityDataProduct:
    def __init__(self, dataProductID=None, data=None):
        self.__gdp = GravityDataProductPB()
        if (dataProductID is not None and data is not None) or (dataProductID is None and data is None):
            raise AttributeError("Exactly one of [dataProductID, data] must be specified")
        if dataProductID is not None:
            self.__gdp.dataProductID = dataProductID
        else:
            try:
                self.__gdp.MergeFromString(data)
            except Exception as e:
                raise AttributeError("Error loading data from given GravityDataProduct; \nCaused by:\n {}".format(traceback.format_exc(e)))
        
    def getGravityTimestamp(self):
        return self.__gdp.timestamp

    def getReceivedTimestamp(self):
        if self.__gdp.HasField('received_timestamp'):
            return self.__gdp.received_timestamp
        else:
            return 0
        
    def setTimestamp(self, ts):
        self.__gdp.timestamp = ts
        
    
    
    
    
    