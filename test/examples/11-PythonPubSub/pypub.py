
import time
import gravity
from gravity import GravityNode, GravityDataProduct, gravity
from BasicCounterDataProduct_pb2 import BasicCounterDataProductPB

gn = GravityNode()
while gn.init("PyPub") != gravity.SUCCESS:
    print "failed to init, retrying..."
    time.sleep(1)
gn.registerDataProduct("PythonGDP", gravity.TCP)

counterPB = BasicCounterDataProductPB()
counterPB.count = 1

gdp = GravityDataProduct("PythonGDP")
gdp.setData(counterPB)
print "gdp = " + str(gdp)

gn.publish(gdp)

gn.waitForExit()
