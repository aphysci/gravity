
import gc, sys
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, GravityRequestor, GravityServiceProvider, Log
from PythonTestPB_pb2 import PythonTestPB

###
### Classes for various handlers
###
class MySubscriber(GravitySubscriber):
    def __init__(self):
        super(MySubscriber, self).__init__()
        self.subCount = 0

    def subscriptionFilled(self, dataProducts):
        testPB = PythonTestPB()
        for gdp in dataProducts:
            gdp.populateMessage(testPB)
            Log.message("received counter with value = "+str(testPB.count))
            self.subCount = testPB.count

class MyRequestHandler(GravityRequestor):
    def __init__(self, gravityNode):
        super(MyRequestHandler, self).__init__()
        self.gravityNode = gravityNode 
        self.reqCount = 0

    def requestFilled(self, serviceID, requestID, response):
        testPB = PythonTestPB()
        response.populateMessage(testPB)
        Log.message("made it to request filled with request GDP ID = "+response.getDataProductID() +" and count = " + str(testPB.count))
        self.reqCount = testPB.count
        if self.reqCount < 5:
            gdp = GravityDataProduct("ServiceRequest")
            gdp.setData(testPB)
            self.gravityNode.request("ServiceTest", gdp, self)
            
class TestProvider(GravityServiceProvider):
    def request(self, serviceID, dataProduct):
        reqPB = PythonTestPB()
        dataProduct.populateMessage(reqPB)
        reqPB.count += 1
        gdp = GravityDataProduct("ServiceTestResponse")
        gdp.setData(reqPB)
        return gdp


###
### Test functions
###
def testPubSub(gravityNode):
    gravityNode.registerDataProduct("PubTest", gravity.TCP)
    mySub = MySubscriber()
    gravityNode.subscribe("PubTest", mySub)
    
    pubPB = PythonTestPB()
    pubPB.count = 0
    pubPB.message = ""
    gdp = GravityDataProduct("PubTest")
    while mySub.subCount < 5 and pubPB.count < 10:
        pubPB.count += 1
        gdp.setData(pubPB)
        gravityNode.publish(gdp)
        time.sleep(1)
        
    if mySub.subCount < 5:
        Log.critical("Pub/Sub failed")
        return 1
    return 0

def testService(gravityNode):
    testProv = TestProvider()
    gravityNode.registerService("ServiceTest", gravity.TCP, testProv)
    
    myReq = MyRequestHandler(gravityNode)
    testPB = PythonTestPB()
    testPB.count = 0
    gdp = GravityDataProduct("ServiceRequest")
    gdp.setData(testPB)
    gravityNode.request("ServiceTest", gdp, myReq)

    # test async
    loopCount = 0
    while myReq.reqCount < 5 and loopCount < 5:
        loopCount += 1
        time.sleep(1)
         
    if myReq.reqCount < 5:
        Log.critical("Asynchronous Service Request failed, expected {} on counter, but was {}".format(5, myReq.reqCount))
        return 1
    
    # test sync
    testPB.count = 0
    gdp.setData(testPB)
    responsePB = PythonTestPB()
    for i in range(0, 5):
        responseGDP = gravityNode.request("ServiceTest", gdp)
        responseGDP.populateMessage(responsePB)
        if responsePB.count != testPB.count+1:
            Log.critical("Incorrect return value, got {} but expected {}".format(responsePB.count, testPB.count+1))
            return 1
        else:
            Log.message("Received return value {} on synchronous request".format(responsePB.count))
        testPB.count += 1
        gdp.setData(testPB)
    
def main():
    gravityNode = GravityNode()
    count = 0
    while gravityNode.init("PythonTest") != gravity.SUCCESS and count < 5:
        Log.warning("failed to init, retrying...")
        time.sleep(1)
        count += 1
    if count == 5:
        Log.critical("Could not connect to ServiceDirectory")
        return 1
    
    ret = testPubSub(gravityNode)
    if ret != 0:
        return ret
    ret = testService(gravityNode)
    if ret != 0:
        return ret
         
    return 0

if __name__ == "__main__":
    sys.exit(main())
