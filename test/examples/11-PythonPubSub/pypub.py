
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity, GravitySubscriber
from BasicCounterDataProduct_pb2 import BasicCounterDataProductPB

class MySubscriber(GravitySubscriber):
    def subscriptionFilled(self, *args):
        print "in my sub filled!"


mySub = MySubscriber()

gn = GravityNode()
while gn.init("PyPub") != gravity.SUCCESS:
    print "failed to init, retrying..."
    time.sleep(1)
gn.registerDataProduct("PythonGDP", gravity.TCP)
gn.subscribe("PythonGDP", mySub)

counterPB = BasicCounterDataProductPB()
counterPB.count = 1

gdp = GravityDataProduct("PythonGDP")
gdp.setData(counterPB)
print "gdp = " + str(gdp)

gn.publish(gdp)

gn.waitForExit()
