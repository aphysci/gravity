
import time

from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber, Log
from BasicCounterDataProduct_pb2 import BasicCounterDataProductPB


class MySubscriber(GravitySubscriber):
    # We're only using GravitySubscriber here, but this is generally the way we 
    # initialize Gravity Python components to ensure that __init__ is called
    # on each parent.
    def __init__(self):
        GravitySubscriber.__init__(self)

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

counterPB = BasicCounterDataProductPB()
gdp = GravityDataProduct("PythonGDP")
for i in range (1, 50):
    counterPB.count = i
    gdp.data=counterPB
    gn.publish(gdp)
    time.sleep(1)
