
import gc, sys
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, GravityRequestor, GravityServiceProvider, GravityHeartbeatListener, Log
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
        self.timeoutCount = 0

    def requestFilled(self, serviceID, requestID, response):
        testPB = PythonTestPB()
        response.populateMessage(testPB)
        Log.message("made it to request filled with request GDP ID = "+response.dataProductID +" and count = " + str(testPB.count))
        self.reqCount = testPB.count
        if self.reqCount < 5:
            gdp = GravityDataProduct("ServiceRequest")
            gdp.data = testPB
            self.gravityNode.request("ServiceTest", gdp, self)
            
    def requestTimeout(self, serviceID, requestID):
        Log.message("Service request timed out")
        self.timeoutCount += 1
            
class TestProvider(GravityServiceProvider):
    def request(self, serviceID, dataProduct):
        reqPB = PythonTestPB()
        dataProduct.populateMessage(reqPB)
        reqPB.count += 1
        gdp = GravityDataProduct("ServiceTestResponse")
        gdp.data = reqPB
        return gdp

class TestHBListener(GravityHeartbeatListener):
    def __init__(self):
        super(TestHBListener, self).__init__()
        self.missedCount = 0
        self.receivedCount = 0
        
    def MissedHeartbeat(self, componentID, microsecond_to_last_heartbeat, interval_in_microseconds):
        Log.message("HB Listener MissedHeartbeat called")
        self.missedCount += 1
    
    def ReceivedHeartbeat(self, componentID, interval_in_microseconds):
        Log.message("HB Listener ReceivedHeartbeat called")
        self.receivedCount += 1
    

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
        gdp.data = pubPB
        gravityNode.publish(gdp)
        time.sleep(.1)
        
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
    gdp.data = testPB
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
    gdp.data = testPB
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
        gdp.data = testPB
    
    return 0
    
def testHB(gravityNode):
    hbListener = TestHBListener()
    gravityNode.registerHeartbeatListener("PythonTest", 100000, hbListener)
    gravityNode.startHeartbeat(100000) # .1 seconds
    count = 0
    while count < 10 and hbListener.receivedCount < 5:
        count += 1
        time.sleep(.1)
    if hbListener.receivedCount < 5:
        Log.critical("didn't receive enough heartbeats. Expected {}, but received {}".format(5, hbListener.receivedCount))
        return 1
    else:
        Log.message("Received {} heartbeats (needed {}, but more is OK)".format(hbListener.receivedCount, 5))
    gravityNode.stopHeartbeat()

    count = 0
    while count < 10 and hbListener.missedCount < 5:
        count += 1
        time.sleep(.1)
    if hbListener.missedCount < 5:
        Log.critical("didn't miss enough heartbeats. Expected {}, but received {}".format(5, hbListener.missedCount))
        return 1
    else:
        Log.message("Missed {} heartbeats (needed {}, but more is OK)".format(hbListener.missedCount, 5))

    gravityNode.unregisterHeartbeatListener("PythonTest")
    
    return 0

def createTempService():
    tempGravityNode = GravityNode()
    count = 0
    while tempGravityNode.init("TempNode") != gravity.SUCCESS and count < 5:
        Log.warning("failed to init, retrying...")
        time.sleep(1)
        count += 1
    if count == 5:
        Log.critical("Could not connect to ServiceDirectory")
        return 1
    testProv = TestProvider()
    tempGravityNode.registerService("TempService", gravity.TCP, testProv)

    time.sleep(2) # give the registration time to complete
    
    return 0

def testServiceTimeout(gravityNode):
    createTempService()
    
    myReq = MyRequestHandler(gravityNode)
    testPB = PythonTestPB()
    testPB.count = 0
    gdp = GravityDataProduct("ServiceRequest")
    gdp.data = testPB
    gravityNode.request("TempService", gdp, myReq, "", 2000)
    
    time.sleep(3)
    
    if myReq.timeoutCount == 0:
        return 1
    return 0
    
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
    ret = testHB(gravityNode)
    if ret != 0:
        return ret
    ret = testServiceTimeout(gravityNode)
    if ret != 0:
        return ret
        
    Log.message("Python tests successful!")
    return 0

if __name__ == "__main__":
    sys.exit(main())
