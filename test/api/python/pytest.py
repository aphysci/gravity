
import gc
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, Log
from PythonTestPB_pb2 import PythonTestPB

class MySubscriber(GravitySubscriber):
    # Don't need to declare your own __init__, but make sure to call 
    # GravitySubscriber.__init__ (super) - therein lies magical SWIG code
    def __init__(self):
        super(MySubscriber, self).__init__()

    def subscriptionFilled(self, dataProducts):
        counterPB = BasicCounterDataProductPB()
        for gdp in dataProducts:
            gdp.populateMessage(counterPB)
            Log.message("received counter with value = "+str(counterPB.count))


mySub = MySubscriber()

gn = GravityNode()
while gn.init("PyPub") != gravity.SUCCESS:
    Log.warning("failed to init, retrying...")
    time.sleep(1)
gn.registerDataProduct("PythonGDP", gravity.TCP)
gn.subscribe("PythonGDP", mySub)


