
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, Log
from BasicCounterDataProduct_pb2 import BasicCounterDataProductPB

class MySubscriber(GravitySubscriber):
    # Don't need to declare your own __init__, but make sure to call 
    # GravitySubscriber.__init__ (super) - therein lies magical SWIG code
    def __init__(self):
        super(MySubscriber, self).__init__()

    def subscriptionFilled(self, dataProducts):
        Log.message("in my sub filled!")
        counterPB = BasicCounterDataProductPB()
        for gdp in dataProducts:
            Log.message(str(type(gdp)))
            Log.message(str(gdp))
            Log.message(str(type(gdp.getDataProductID())))
            Log.message(gdp.getDataProductID())
            gdp.populateMessage(counterPB)
            Log.message("receive counter with value = "+str(counterPB.count))


mySub = MySubscriber()

gn = GravityNode()
while gn.init("PyPub") != gravity.SUCCESS:
    Log.warning("failed to init, retrying...")
    time.sleep(1)
gn.registerDataProduct("PythonGDP", gravity.TCP)
gn.subscribe("PythonGDP", mySub)

counterPB = BasicCounterDataProductPB()
gdp = GravityDataProduct("PythonGDP")
for i in range (1, 50):
    counterPB.count = i
    gdp.setData(counterPB)
    gn.publish(gdp)
    time.sleep(1)

gn.waitForExit()
