
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravityRequestor, GravityServiceProvider, Log
from Multiplication_pb2 import MultiplicationOperandsPB, MultiplicationResultPB

done = False
class MyRequestorProvider(GravityRequestor, GravityServiceProvider):
    # We need to explicitly initialize each parent here to allow swig to
    # configure the underlying proxy object correctly.
    def __init__(self):
        GravityRequestor.__init__(self)
        GravityServiceProvider.__init__(self)

    def requestFilled(self, serviceID, requestID, response):
        multResponse = MultiplicationResultPB()
        response.populateMessage(multResponse)
        Log.message("made it to request filled with request GDP ID = "+response.dataProductID +" and response = " + str(multResponse.result))
        global done
        done = True

    def request(self, serviceID, dataProduct):
        Log.message("made it to my request!")
        Log.message("for serviceID = "+serviceID)
        operands = MultiplicationOperandsPB()
        Log.message(str(type(operands)))
        dataProduct.populateMessage(operands)
        Log.message("have operands = "+str([operands.multiplicand_a, operands.multiplicand_b]))

        multResponse = MultiplicationResultPB()
        multResponse.result = operands.multiplicand_a * operands.multiplicand_b
        gdp = GravityDataProduct("MultResponse")
        gdp.data = multResponse
        Log.message("returning response with result = "+str(multResponse.result))
        return gdp

gn = GravityNode()
while gn.init("PyRequest") != gravity.SUCCESS:
    Log.warning("failed to init, retrying...")
    time.sleep(1)

requestorProvider = MyRequestorProvider()
gn.registerService("Multiplication", gravity.TCP, requestorProvider)

# Async request
operands = MultiplicationOperandsPB()
operands.multiplicand_a = 3
operands.multiplicand_b = 4
gdp = GravityDataProduct("MultRequest")
gdp.data = operands
gn.request("Multiplication", gdp, requestorProvider)

while not done:
    time.sleep(1)

# Sync request
operands.multiplicand_a = 5
operands.multiplicand_b = 6
gdp.data=operands
gdpResp = gn.request("Multiplication", gdp)
Log.message("received GDP response")
multResponse = MultiplicationResultPB()
gdpResp.populateMessage(multResponse)
Log.message("made it to request filled with request GDP ID = "+gdpResp.dataProductID +" and response = " + str(multResponse.result))




