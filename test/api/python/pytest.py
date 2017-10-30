
import gc, sys
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, GravityRequestor, GravityServiceProvider, Log
from PythonTestPB_pb2 import PythonTestPB

testCount = 0

class MySubscriber(GravitySubscriber):
    def subscriptionFilled(self, dataProducts):
        testPB = PythonTestPB()
        for gdp in dataProducts:
            gdp.populateMessage(testPB)
            Log.message("received counter with value = "+str(testPB.count))
            global testCount
            testCount = testPB.count

class MyRequestHandler(GravityRequestor):
    def __init__(self, gravityNode):
        super(MyRequestHandler, self).__init__()
        self.gravityNode = gravityNode 

    def requestFilled(self, serviceID, requestID, response):
        testPB = PythonTestPB()
        response.populateMessage(testPB)
        Log.message("made it to request filled with request GDP ID = "+response.getDataProductID() +" and count = " + str(testPB.count))
        global testCount
        testCount = testPB.count
        if testCount < 5:
            gdp = GravityDataProduct("ServiceRequest")
            gdp.setData(testPB)
            self.gravityNode.request("ServiceTest", gdp, self)
            

class TestProvider(GravityServiceProvider):
    def request(self, serviceID, dataProduct):
        testPB = PythonTestPB()
        dataProduct.populateMessage(testPB)
        testPB.count += 1
        gdp = GravityDataProduct("ServiceTestResponse")
        gdp.setData(testPB)
        return gdp

def testPubSub(gravityNode):
    gravityNode.registerDataProduct("PubTest", gravity.TCP)
    mySub = MySubscriber()
    gravityNode.subscribe("PubTest", mySub)
    
    pubPB = PythonTestPB()
    pubPB.count = 0
    pubPB.message = ""
    gdp = GravityDataProduct("PubTest")
    global testCount
    while testCount < 5 and pubPB.count < 10:
        pubPB.count += 1
        gdp.setData(pubPB)
        gravityNode.publish(gdp)
        time.sleep(1)
        
    if testCount < 5:
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

    global testCount
    testCount = 0
    loopCount = 0
    while testCount < 5 and loopCount < 5:
        loopCount += 1
        time.sleep(1)
        
    if testCount < 5:
        Log.critical("Service Request failed")
        return 1
    
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
