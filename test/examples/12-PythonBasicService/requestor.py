
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravityRequestor, GravityServiceProvider, SpdLog
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
        SpdLog.warn("made it to request filled with request GDP ID = "+response.dataProductID +" and response = " + str(multResponse.result))
        global done
        done = True

    def request(self, serviceID, dataProduct):
        SpdLog.warn("made it to my request!")
        SpdLog.warn("for serviceID = "+serviceID)
        operands = MultiplicationOperandsPB()
        SpdLog.warn(str(type(operands)))
        dataProduct.populateMessage(operands)
        SpdLog.warn("have operands = "+str([operands.multiplicand_a, operands.multiplicand_b]))

        multResponse = MultiplicationResultPB()
        multResponse.result = operands.multiplicand_a * operands.multiplicand_b
        gdp = GravityDataProduct("MultResponse")
        gdp.data = multResponse
        SpdLog.warn("returning response with result = "+str(multResponse.result))
        return gdp

gn = GravityNode()
while gn.init("PyRequest") != gravity.SUCCESS:
    SpdLog.warn("failed to init, retrying...")
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
SpdLog.warn("received GDP response")
multResponse = MultiplicationResultPB()
gdpResp.populateMessage(multResponse)
SpdLog.warn("made it to request filled with request GDP ID = "+gdpResp.dataProductID +" and response = " + str(multResponse.result))




