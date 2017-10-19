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

from google.protobuf import message
from GravityDataProductPB_pb2 import GravityDataProductPB

class GravityDataProduct:
    def __init__(self, dataProductID=None, data=None):
        self.__gdp = GravityDataProductPB()
        if (dataProductID is not None and data is not None) or (dataProductID is None and data is None):
            raise AttributeError("Exactly one of [dataProductID, data] must be specified")
        if dataProductID is not None:
            self.__gdp.dataProductID = str(dataProductID)
        else:
            try:
                self.__gdp.MergeFromString(data)
            except Exception as e:
                raise AttributeError("Error loading GravityDataProduct from given data; \nCaused by:\n {}".format(traceback.format_exc(e)))
        
    def getGravityTimestamp(self):
        return self.__gdp.timestamp

    def getReceivedTimestamp(self):
        if self.__gdp.HasField('received_timestamp'):
            return self.__gdp.received_timestamp
        else:
            return 0
        
    def setTimestamp(self, ts):
        self.__gdp.timestamp = ts
        
    def getDataProductID(self):
        return str(self.__gdp.dataProductID)
    
    def setSoftwareVersion(self, sv):
        self.__gdp.softwareVersion = str(sv)
        
    def getSoftwareVersion(self):
        return str(self.__gdp.softwareVersion)
    
    def setData(self, data):
        if isinstance(data, message.Message):
            self.__gdp.data = data.SerializeToString()
        elif isinstance(data, bytearray) or isinstance(data, bytes) or isinstance(data, str):
            self.__gdp.data = str(data)
        else:
            raise AttributeError("Invalid type for data ({}) - must be a Protobuf or a bytearray/bytes/str".format(type(data)))
        
    def getData(self):
        if self.__gdp.data is None:
            return None
        return str(self.__gdp.data)
    
    def getDataSize(self):
        if self.__gdp.data is None:
            return 0
        return len(self.__gdp.data)
    
    def populateMessage(self, data):
        try:
            data.MergeFrom(self.__gdp.data)
        except Exception as e:
            raise AttributeError("Error populating given Protobuf; \nCaused by:\n {}".format(traceback.format_exc(e)))
        
    def getSize(self):
        return len(self.__gdp.SerializeToString())
        
    def parseFromString(self, data):
        try:
            self.__gdp.MergeFromString(data)
        except Exception as e:
            raise AttributeError("Error loading GravityDataProduct from given data; \nCaused by:\n {}".format(traceback.format_exc(e)))

    def serializeToString(self):
        return self.__gdp.SerializeToString()
    
    def setComponentID(self, componentID):
        self.__gdp.componentID = str(componentID)
        
    def getComponentID(self):
        return str(self.__gdp.componentID)
    
    def setDomain(self, domain):
        self.__gdp.domain = str(domain)
        
    def getDomain(self):
        return str(self.__gdp.domain)
    
    def getFutureSocketUrl(self):
        return str(self.__gdp.future_socket_url)
    
    def isFutureResponse(self):
        return self.__gdp.future_response
    
    def isCachedDataproduct(self):
        return self.__gdp.HasField('is_cached_dataproduct') and self.__gdp.is_cached_dataproduct
    
    def setIsCachedDataproduct(self, isCachedDataproduct):
        self.__gdp.is_cached_dataproduct = isCachedDataproduct
    
    