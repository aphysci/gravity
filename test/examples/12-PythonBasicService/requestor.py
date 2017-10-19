
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravityRequestor, Log
from BasicCounterDataProduct_pb2 import BasicCounterDataProductPB

class MyRequestor(GravityRequestor):
    def requestFilled(self, serviceID, requestID, dataProducts):
        Log.message("made it to request filled!")

gn = GravityNode()
while gn.init("PyPub") != gravity.SUCCESS:
    Log.warning("failed to init, retrying...")
    time.sleep(1)

myReq = MyRequestor()

gdp = GravityDataProduct("MyRequest")
gn.request("MyService", gdp, myReq)

gn.waitForExit()
